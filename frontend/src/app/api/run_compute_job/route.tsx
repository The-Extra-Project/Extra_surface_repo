"use server"
import { NextRequest, NextResponse } from "next/server";
import { configDotenv } from "dotenv";
import { resolve } from "path";
  
export interface scheduleJob {
    input_url: string,
    username: string

}



configDotenv(
    {
        path: resolve(__dirname, "../../../.env")
    }
)


export async function POST(request: NextRequest) {

try {

    const {filepath, email} = await request.json();
    // calling the previous recursive api
    let jobParams: scheduleJob = {
        input_url: filepath,
        username:  email
    };  
     const response = await fetch( process.env.API_SERVER_URL + "/reconstruction/schedule?file_path=" + jobParams.input_url + "&email=" + jobParams.username, {
        method: "POST",
        body: ""
    })
    return await response.json()
}
catch(error) {

    console.error("/api/run_compute_job" + error)
}

}