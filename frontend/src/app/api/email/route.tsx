'use server'

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
  const data = await req.json()
  const formData = await req.formData();
  const email = formData.get("email") as File;
  const jobId = formData.get("jobId") as string;

  try {

    const {email, jobId} = data as {email: string, jobId: string}
      const response =  await fetch( process.env.API_SERVER_URL +  "/email/send_mail_paid" , {
        method: "POST",
        body: JSON.stringify(
            {
                "receiever_email": email as string,
                "payment_reference": jobId as string

            }
        )
    })
    return Response.json(
      {
        "result": response
      }
    )
    }
    catch(error) {
      console.error("error in /api/email"+ error);
      return Response.json(
        {
          "error": "error in /api/email" + error
        }
      );     
    }
}