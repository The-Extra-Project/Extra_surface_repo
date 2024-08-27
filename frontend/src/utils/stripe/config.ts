import Stripe from 'stripe';
import { config } from "dotenv";
import path, {resolve} from 'path';
import { env } from '@/env';


export const stripe = new Stripe(  
  env.STRIPE_SECRET_KEY,
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
