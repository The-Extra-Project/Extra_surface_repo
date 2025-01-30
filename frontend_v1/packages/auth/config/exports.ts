import {env} from "@repo/env";
import { ResourcesConfig } from "aws-amplify";

export const oauthConfig = {
    domain: process.env.OAUTH_DOMAIN,
    scope: ['email', 'profile', 'openid'],
    redirectSignIn: process.env.OAUTH_REDIRECT_SIGN_IN,
    redirectSignOut: process.env.OAUTH_REDIRECT_SIGN_OUT,
    responseType: 'code',
  };
  


export const authConfig: ResourcesConfig = {
    Auth: {
        region: env.AWS_PROJECT_REGION,
        userPoolId: env.AWS_USER_POOLS_ID,
        userPoolWebClientId: env.AWS_USER_POOLS_WEB_CLIENT_ID,
        identityPoolId: env.AWS_COGNITO_IDENTITY_POOL_ID,
        oauth: oauthConfig,
      },
      API: {
        aws_appsync_graphqlEndpoint: env.AWS_APPSYNC_GRAPHQL_ENDPOINT,
        aws_appsync_region: env.AWS_APPSYNC_REGION,
        aws_appsync_authenticationType: env.AWS_APPSYNC_AUTHENTICATION_TYPE,
      },
};

/**
 * aws_cognito_identity_pool_id: The ID of your Cognito Identity Pool2.
aws_cognito_region: The region where your Cognito resources are located2.
aws_user_pools_id: The ID of your Cognito User Pool2.
aws_user_pools_web_client_id: The client ID for your Cognito User Pool2.
oauth: An object containing OAuth-related configurations, if you're using social sign-in providers2.
aws_appsync_graphqlEndpoint: The endpoint URL for your AppSync API, if you're using GraphQL1.
aws_appsync_region: The region where your AppSync API is located1.
aws_appsync_authenticationType: The authentication type for your AppSync API1.

 */