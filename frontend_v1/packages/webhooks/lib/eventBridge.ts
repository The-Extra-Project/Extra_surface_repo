import 'server-only';
import { auth } from '@repo/auth/server';
import { env } from '@repo/env';
import { EventBridgeClient, PutEventsCommand, ListRulesCommand, ListTargetsByRuleCommand } from "@aws-sdk/client-eventbridge";


//TODO: currently these are defined as an temporary parameters and it will bound to be changed. 
export interface sendS3Notification {
  s3_uri: string,
  username: string

}

export interface SendEventMessageType {
  sourceBridge: string,
  eventType: string
  payloadObject: sendS3Notification 
}

const eventbridge = new EventBridgeClient({
  region: env.AWS_REGION,
  credentials: {
    accessKeyId: env.AWS_ACCESS_KEY_ID,
    secretAccessKey: env.AWS_SECRET_ACCESS_KEY,
  },
});

/**
 * sends the event request to bridge for commencing backend operations
 * this can be done when the file is uploaded and the client schedules the operation (after paying the fees).
 * @param input_params is the parameters defining to which source event bridge is to ba
 * 
 * @returns the response to the send API call
 */
export const sendEvent = async (input_params: SendEventMessageType): Promise<any> => {
  const { orgId } = await auth();

  if (!orgId) {
    return;
  }

  let eventType = input_params.eventType
  const params = {
    Entries: [
      {
        Source: input_params.sourceBridge,
        DetailType: input_params.eventType,
        Detail: JSON.stringify({
          eventType,
          ...input_params.payloadObject,
        }),
        EventBusName: 'default',
      },
    ],
  };

  try {
    const data = await eventbridge.send(new PutEventsCommand(params));
    return data;
  } catch (err) {
    console.error(err);
    throw new Error('Failed to send event to EventBridge');
  }
};

export const getNotifications = async () => {
  const { orgId } = await auth();

  if (!orgId) {
    return;
  }

  try {
    const rules = await eventbridge.send(new ListRulesCommand({}));

    const targets = rules.Rules
    ? await Promise.all(
        rules.Rules.map(async (rule) => {
          const targetData = await eventbridge.send(new ListTargetsByRuleCommand({ Rule: rule.Name }));
          return targetData.Targets;
        })
      )
    : [];

    return targets.flat();
  } catch (err) {
    console.error(err);
    throw new Error('Failed to fetch events from EventBridge');
  }
};
