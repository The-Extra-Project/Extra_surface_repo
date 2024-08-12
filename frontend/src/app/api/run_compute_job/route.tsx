"use server"
import { NextRequest, NextResponse } from "next/server";

export interface scheduleJob {
    input_url: string,
    username: string

}

import { configDotenv } from "dotenv";
import { resolve } from "path";


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
     const response = await fetch(   process.env.API_SERVER_URL! + "/reconstruction/schedule" , {
        body: JSON.stringify(
            {
                jobParams
            }
        )
    })
    return await response.json()
}
catch(error) {

    console.error("/api/run_compute_job" + error)
}

}