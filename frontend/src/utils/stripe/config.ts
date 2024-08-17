import Stripe from 'stripe';
import { config, configDotenv } from "dotenv";
import path, {resolve} from 'path';
configDotenv(
  {
    path: resolve(__dirname + "../../.env")
  }
)
export const stripe = new Stripe(
  process.env.STRIPE_PUBLIC_KEY!,
    {
        // @ts-ignore
        apiVersion: null,
        appInfo: {
          name: 'Extra Surface',
          version: '0.0.1',
          url: 'https://github.com/The-Extra-Project/Extra-Surface',
        }
      }
)
