import Stripe from 'stripe';
import { config, configDotenv } from "dotenv";
import path from 'path';
import { env } from 'src/env';

export const stripe = new Stripe(
  "sk_test_51PGIH1RxsgXWpWf5YJ2bv3cvNrxBQe5RipRW7xqu95upcYpWtzqVdyGzd3ZINOoLu86qOPeJG5mAgmsiTRQp8uZw00Lp5Hu9NT",
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