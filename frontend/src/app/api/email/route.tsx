'use server'
import { NextApiRequest, NextApiResponse } from 'next';
import { Resend } from 'resend';
import { configDotenv } from 'dotenv';
import {resolve} from "path"
import loadConfig from 'next/dist/server/config';

configDotenv(
  {
    path: resolve(__dirname, "../../../.env")
  }
)

const resend = new Resend(process.env.RESEND_API_KEY!.toString());

interface emailStr {
  email: string;
  cost: number;
  URLs: string;
}

export async function POST(req: Request) {
    const { email, cost, URLs }: emailStr = req.body as unknown as emailStr;
    try {
        const { data, error } = await resend.emails.send({
        from: 'Extralabs <hello@extralabs.xyz>',
        to: email,
        subject: 'Job submitted successfully',
        text: `Thanks for scheduling the job for Extra-Surface, 
        Just to give you the recapitulative
        the cost of the job is ${cost}
        and the tiles to be reconstructed are ${URLs}
        we will be sending you the link of the storage and reconstructed files once its completed
        dont hesitate to reach out to us for any queries.
        `,
      });
      return Response.json(data);
    } catch (error) {
      console.error('Error sending email:', error);
      return Response.json({ error: 'Error sending email' });
    } 
}