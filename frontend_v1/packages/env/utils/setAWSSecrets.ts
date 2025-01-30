import { SecretsManagerClient, PutSecretValueCommand } from "@aws-sdk/client-secrets-manager";
import { env } from '../index';
const client = new SecretsManagerClient({ region: "us-east-1" });

export const setAWSSecrets = async (secretName: string, accessKeyId: string, secretAccessKey: string) => {
    try {
        const command = new PutSecretValueCommand({
            SecretId: secretName,
            SecretString: JSON.stringify({
                AWS_ACCESS_KEY_ID: accessKeyId,
                AWS_SECRET_ACCESS_KEY: secretAccessKey
            })
        });
        await client.send(command);
        console.log("Secrets set successfully");
    } catch (error) {
        console.error("Error setting secrets:", error);
        throw error;
    }
};
const initializeSecrets = async () => {
    await setAWSSecrets(
        "my-secret-name",
        env.AWS_ACCESS_KEY_ID,
        env.AWS_SECRET_ACCESS_KEY
    );
};

initializeSecrets().catch(console.error);

const initializeAllSecrets = async () => {
    const secrets = {
        AWS_REGION: env.AWS_REGION,
        AWS_ACCESS_KEY_ID: env.AWS_ACCESS_KEY_ID,
        AWS_SECRET_ACCESS_KEY:env.AWS_SECRET_ACCESS_KEY,
        // Add other secrets from the env variable here 
        AWS_COGNITO_IDENTITY_POOL_ID: env.AWS_COGNITO_IDENTITY_POOL_ID,
        AWS_USER_POOLS_ID: env.AWS_USER_POOLS_ID,
        AWS_USER_POOLS_WEB_CLIENT_ID: env.AWS_USER_POOLS_WEB_CLIENT_ID,
        AWS_APPSYNC_GRAPHQL_ENDPOINT: env.AWS_APPSYNC_GRAPHQL_ENDPOINT,
        AWS_APPSYNC_REGION: env.AWS_APPSYNC_REGION,
        AWS_APPSYNC_AUTHENTICATION_TYPE: env.AWS_APPSYNC_AUTHENTICATION_TYPE,
        NEXT_PUBLIC_OAUTH_DOMAIN: env.NEXT_PUBLIC_OAUTH_DOMAIN,
    };
    for (const [key, value] of Object.entries(secrets)) {
        if (value) {
            await setAWSSecrets(key, value, value);
        }
    }
};
initializeAllSecrets().catch(console.error);