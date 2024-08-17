"use server"
import fs from "fs/promises"
import { NextRequest } from "next/server";
import {resolve} from "path"
import {  uploadCommandS3 } from "src/utils/s3_files_auth";
import {supabaseClient} from "src/utils/supabase_server"
import { Tables } from "src/utils/types_db";

const uuid = require("uuid")


export async function POST(req:Request) {
    //const {file, email} : {file: File, email:string} = await req.json()
    const formData = await req.formData();
    const file = formData.get("file") as File;
    const email = formData.get("email") as string;

    try {
    const arrayBuffer = await file.arrayBuffer();
    const buffer = new Uint8Array(arrayBuffer);
    
    // const buffer = new Uint8Array(fileBuffer);
    var key_value = email
    const upload_command = uploadCommandS3(buffer, key_value)
    await supabaseClient.from("job_command_surface").insert({
        email: email,
        time: (new Date().getTime()).toString(10),
        file: (await upload_command).location
    })   
    
    return Response.json({
        location: (await upload_command).location,
        upload_id: (await upload_command).upload_id
    })

}
    catch(Error) {
        console.error("/api/files not working: " + Error)
    }
    return Response.json({
        location:"",
        upload_id: ""
    })
}