import { createEnv } from '@t3-oss/env-nextjs';
import { z } from 'zod';
import { SecretsManagerClient, GetSecretValueCommand } from "@aws-sdk/client-secrets-manager";

async function getAwsCredentials() {
  const secretsManager = new SecretsManagerClient({
    region: 'us-east-1'
  })
  const keyIdParam =  new GetSecretValueCommand({
    SecretId: "AWS_ACCESS_KEY_ID"
  })   

  const secret = new GetSecretValueCommand(
    {
      SecretId:"AWS_SECRET_ACCESS_KEY"
    }
  )

  const keyResponse = await secretsManager.send(keyIdParam)
  const secretResponse = await secretsManager.send(secret)

  if(!keyResponse || !secretResponse) {
    throw new Error("Secret not found");
  }
  return {
    AWS_REGION: 'us-east-1',
    AWS_ACCESS_KEY_ID: keyResponse.SecretString!,
    AWS_SECRET_ACCESS_KEY: keyResponse.SecretString!,
  };
}

const server: Parameters<typeof createEnv>[0]['server'] = {
  CLERK_SECRET_KEY: z.string().min(1).startsWith('sk_'),
  CLERK_WEBHOOK_SECRET: z.string().min(1).startsWith('whsec_').optional(),
  RESEND_FROM: z.string().min(1).email(),
  DATABASE_URL: z.string().min(1).url(),
  RESEND_TOKEN: z.string().min(1).startsWith('re_'),
  STRIPE_SECRET_KEY: z.string().min(1).startsWith('sk_'),
  STRIPE_WEBHOOK_SECRET: z.string().min(1).startsWith('whsec_').optional(),
  BETTERSTACK_API_KEY: z.string().min(1).optional(),
  BETTERSTACK_URL: z.string().min(1).url().optional(),
  ARCJET_KEY: z.string().min(1).startsWith('ajkey_').optional(),
  ANALYZE: z.string().optional(),
  LIVEBLOCKS_SECRET: z.string().min(1).startsWith('sk_').optional(),
  OPENAI_API_KEY: z.string().min(1).startsWith('sk-').optional(),
  BASEHUB_TOKEN: z.string().min(1).startsWith('bshb_pk_'),
  UPSTASH_REDIS_REST_URL: z.string().min(1).url().optional(),
  UPSTASH_REDIS_REST_TOKEN: z.string().min(1).optional(),
  AWS_REGION: z.string().min(1),
  AWS_ACCESS_KEY_ID: z.string().min(1),
  AWS_SECRET_ACCESS_KEY: z.string().min(1),
  ANTHROPIC_KEY: z.string().optional(),

  // Added by Sentry Integration, Vercel Marketplace
  SENTRY_ORG: z.string().min(1).optional(),
  SENTRY_PROJECT: z.string().min(1).optional(),

  // Added by Vercel
  VERCEL: z.string().optional(),
  NEXT_RUNTIME: z.enum(['nodejs', 'edge']).optional(),
  FLAGS_SECRET: z.string().min(1).optional(),
  BLOB_READ_WRITE_TOKEN: z.string().min(1).optional(),
};

const client: Parameters<typeof createEnv>[0]['client'] = {
  NEXT_PUBLIC_CLERK_PUBLISHABLE_KEY: z.string().min(1).startsWith('pk_'),
  NEXT_PUBLIC_CLERK_SIGN_IN_URL: z.string().min(1).startsWith('/'),
  NEXT_PUBLIC_CLERK_SIGN_UP_URL: z.string().min(1).startsWith('/'),
  NEXT_PUBLIC_CLERK_AFTER_SIGN_IN_URL: z.string().min(1).startsWith('/'),
  NEXT_PUBLIC_CLERK_AFTER_SIGN_UP_URL: z.string().min(1).startsWith('/'),
  NEXT_PUBLIC_APP_URL: z.string().min(1).url(),
  NEXT_PUBLIC_WEB_URL: z.string().min(1).url(),
  NEXT_PUBLIC_API_URL: z.string().min(1).url().optional(),
  NEXT_PUBLIC_DOCS_URL: z.string().min(1).url().optional(),
  NEXT_PUBLIC_GA_MEASUREMENT_ID: z.string().min(1).startsWith('G-').optional(),
  NEXT_PUBLIC_POSTHOG_KEY: z.string().min(1).startsWith('phc_'),
  NEXT_PUBLIC_POSTHOG_HOST: z.string().min(1).url(),

  // Added by Vercel
  NEXT_PUBLIC_VERCEL_PROJECT_PRODUCTION_URL: z.string().min(1),
};

const { AWS_REGION, AWS_ACCESS_KEY_ID, AWS_SECRET_ACCESS_KEY } = await getAwsCredentials();

export const env = createEnv({
  client,
  server,
  runtimeEnv:  {
   
      CLERK_SECRET_KEY: process.env.CLERK_SECRET_KEY,
      CLERK_WEBHOOK_SECRET: process.env.CLERK_WEBHOOK_SECRET,
      RESEND_FROM: process.env.RESEND_FROM,
      DATABASE_URL: process.env.DATABASE_URL,
      RESEND_TOKEN: process.env.RESEND_TOKEN,
      STRIPE_SECRET_KEY: process.env.STRIPE_SECRET_KEY,
      STRIPE_WEBHOOK_SECRET: process.env.STRIPE_WEBHOOK_SECRET,
      BETTERSTACK_API_KEY: process.env.BETTERSTACK_API_KEY,
      BETTERSTACK_URL: process.env.BETTERSTACK_URL,
      ARCJET_KEY: process.env.ARCJET_KEY,
      ANALYZE: process.env.ANALYZE,
      SENTRY_ORG: process.env.SENTRY_ORG,
      SENTRY_PROJECT: process.env.SENTRY_PROJECT,
      VERCEL: process.env.VERCEL,
      NEXT_RUNTIME: process.env.NEXT_RUNTIME,
      FLAGS_SECRET: process.env.FLAGS_SECRET,
      BLOB_READ_WRITE_TOKEN: process.env.BLOB_READ_WRITE_TOKEN,
      LIVEBLOCKS_SECRET: process.env.LIVEBLOCKS_SECRET,
      OPENAI_API_KEY: process.env.OPENAI_API_KEY,
      BASEHUB_TOKEN: process.env.BASEHUB_TOKEN,
      AWS_REGION,
      AWS_ACCESS_KEY_ID,
      AWS_SECRET_ACCESS_KEY,
      NEXT_PUBLIC_CLERK_PUBLISHABLE_KEY: process.env.NEXT_PUBLIC_CLERK_PUBLISHABLE_KEY,
      NEXT_PUBLIC_CLERK_SIGN_IN_URL: process.env.NEXT_PUBLIC_CLERK_SIGN_IN_URL,
      NEXT_PUBLIC_CLERK_SIGN_UP_URL: process.env.NEXT_PUBLIC_CLERK_SIGN_UP_URL,
      NEXT_PUBLIC_CLERK_AFTER_SIGN_IN_URL: process.env.NEXT_PUBLIC_CLERK_AFTER_SIGN_IN_URL,
      NEXT_PUBLIC_CLERK_AFTER_SIGN_UP_URL: process.env.NEXT_PUBLIC_CLERK_AFTER_SIGN_UP_URL,
      NEXT_PUBLIC_APP_URL: process.env.NEXT_PUBLIC_APP_URL,
      NEXT_PUBLIC_WEB_URL: process.env.NEXT_PUBLIC_WEB_URL,
      NEXT_PUBLIC_API_URL: process.env.NEXT_PUBLIC_API_URL,
      NEXT_PUBLIC_DOCS_URL: process.env.NEXT_PUBLIC_DOCS_URL,
      NEXT_PUBLIC_GA_MEASUREMENT_ID: process.env.NEXT_PUBLIC_GA_MEASUREMENT_ID,
      NEXT_PUBLIC_POSTHOG_KEY: process.env.NEXT_PUBLIC_POSTHOG_KEY,
      NEXT_PUBLIC_POSTHOG_HOST: process.env.NEXT_PUBLIC_POSTHOG_HOST,
      NEXT_PUBLIC_VERCEL_PROJECT_PRODUCTION_URL: process.env.NEXT_PUBLIC_VERCEL_PROJECT_PRODUCTION_URL,
  },
});