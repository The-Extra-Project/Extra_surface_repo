import Stripe from 'stripe';
import { config, configDotenv } from "dotenv";
import path, {resolve} from 'path';

configDotenv(
  {
    path: resolve(__dirname, "../../.env")
  }
)



export const stripe = new Stripe(
  process.env.STRIPE_PUBLIC_KEY!,
    {
        // https://github.com/stripe/stripe-node#configuration
        // https://stripe.com/docs/api/versioning
        // @ts-ignore
        apiVersion: null,
        // Register this as an official Stripe plugin.
        // https://stripe.com/docs/building-plugins#setappinfo
        appInfo: {
          name: 'Extra Surface',
          version: '0.0.1',
          url: 'https://github.com/The-Extra-Project/Extra-Surface',
        }
      }
)

export const loadStripe = async () : Promise<Stripe | null> => {
    return await loadStripe();
}