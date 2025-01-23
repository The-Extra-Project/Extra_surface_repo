import {Client} from "@repo/auth/server"
import {env} from "@repo/env"
import { stripe } from '@repo/payments';
import  type {Stripe} from '@repo/payments';
import { headers } from 'next/headers';
import { NextRequest,NextResponse } from 'next/server';

export async function POST(request: NextRequest): Promise<NextResponse> {
        try {
          const { amount } = await request.json();
          const paymentIntent = await stripe.paymentIntents.create({
            amount: amount,
            currency: "eur",
            automatic_payment_methods: { enabled: true },
          });
          return NextResponse.json({ clientSecret: paymentIntent.client_secret });
        } catch (error) {
          console.error("Internal Error:", error);
          // Handle other errors (e.g., network issues, parsing errors)
          return NextResponse.json(
            { error: `Internal Server Error: ${error}` },
            { status: 500 }
          );
        }
}