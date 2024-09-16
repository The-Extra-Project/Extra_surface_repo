import { createEnv } from "@t3-oss/env-nextjs";
import {z} from "zod";

export const env = createEnv({
   server: {
      NODE_ENV: z.enum(["development", "production"]),
      STRIPE_SECRET_KEY: z.string(),
      STRIPE_PUBLIC_KEY: z.string(),
      API_SERVER_LAMBDA_FILE_UPLOAD: z.string(),
      API_SERVER_URL: z.string(),
      AWS_REGION: z.string(),
      SUPABASE_URL: z.string(),
      SUPABASE_ANON_KEY: z.string(),
      S3_BUCKET: z.string()
    },
    client: {
      NEXT_PUBLIC_NODE_ENV: z.enum(["development", "production"]),
      NEXT_PUBLIC_STRIPE_PUBLIC_KEY: z.string(),
      NEXT_PUBLIC_WEBSITE_URL: z.string(),
      NEXT_PUBLIC_API_SERVER_URL: z.string()
   },
    runtimeEnv: {
      API_SERVER_LAMBDA_FILE_UPLOAD: process.env.API_SERVER_LAMBDA_FILE_UPLOAD,
      NODE_ENV: process.env.NODE_ENV,
      AWS_REGION: process.env.AWS_REGION,
      STRIPE_PUBLIC_KEY: process.env.STRIPE_PUBLIC_KEY,
      STRIPE_SECRET_KEY: process.env.STRIPE_SECRET_KEY,
      API_SERVER_URL: process.env.API_SERVER_URL,
      SUPABASE_URL: process.env.SUPABASE_URL,
      SUPABASE_ANON_KEY: process.env.SUPABASE_ANON_KEY,
      S3_BUCKET: process.env.S3_BUCKET,
      NEXT_PUBLIC_NODE_ENV: process.env.NEXT_PUBLIC_NODE_ENV,
      NEXT_PUBLIC_STRIPE_PUBLIC_KEY: process.env.NEXT_PUBLIC_STRIPE_PUBLIC_KEY,
      NEXT_PUBLIC_WEBSITE_URL: process.env.NEXT_PUBLIC_WEBSITE_URL,
      NEXT_PUBLIC_API_SERVER_URL: process.env.NEXT_PUBLIC_API_SERVER_URL
    },  
});
