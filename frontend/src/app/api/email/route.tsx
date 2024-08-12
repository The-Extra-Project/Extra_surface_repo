'use server'
import { NextApiRequest, NextApiResponse } from 'next';
import { Resend } from 'resend';
import { configDotenv } from 'dotenv';
import {resolve} from "path"
configDotenv(
  {
    path: resolve(__dirname, "../../../.env")
  }
)

interface emailParams {
  email: string;
  jobId: string;
}

export async function POST(req: Request) {
    const { email, jobId } = req.body as unknown as emailParams;
    try {
      const response =  await fetch( process.env.API_SERVER_URL! +  "/reconstruction/email/send_result" , {
        body: JSON.stringify(
            {
                email,
                jobId
            }
        )
    })

    return response.json()["emailId"]
    }
    catch(error) {
      console.error("error in /api/email");
    }

}