"use server"
import { NextRequest } from "next/server";
import {env} from "@/env"
import {APIGatewayClient, GetAccountCommand } from "@aws-sdk/client-api-gateway"
//import {  uploadCommandS3 } from "src/utils/s3_files_auth";
import {supabaseClient} from "src/utils/supabase_server"

export async function POST(req:Request) {
    const formData = await req.formData();
    const file = formData.get("file") as File;
    const email = formData.get("email") as string;
    const client = new APIGatewayClient(
        {endpoint: env.API_SERVER_LAMBDA_FILE_UPLOAD,region: "us-east-1"}
    );
    const api_params = {  
        inputPath: file.webkitRelativePath,
        username: email 
    }
    try {
    const apiCommand = await new GetAccountCommand(api_params)
    // const buffer = new Uint8Array(arrayBuffer);
    // var key_value = email
    // const upload_command = uploadCommandS3(buffer, key_value)
    const command = client.send(apiCommand)
    await supabaseClient.from("job_command_surface").insert({
        email: email,
        time: (new Date().toLocaleTimeString()),
        file: (await command)["file"]
    })   
    
    return Response.json({
        location: (await command)["stored_path"],
        upload_id: (await command).$metadata.requestId
    })

}
    catch(Error) {
        console.error("/api/files not working: " + Error)
    }
    return Response.json({
        location:"null: error",
        upload_id: "0"
    })
}