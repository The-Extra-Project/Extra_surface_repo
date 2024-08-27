'use server'

import { configDotenv } from 'dotenv';
import {resolve} from "path"
import {env} from "@/env"

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
  const formData = await req.formData();
  const email = formData.get("email") as string;
  const jobId = formData.get("jobId") as string;
  try {
      const response =  await fetch( env.API_SERVER_URL +  "/email/send_mail_paid?receiever_mail=" + email + "&payment_reference=" + jobId, {
        method: "POST",
        body: ''
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