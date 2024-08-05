"use server"
import fs from "fs/promises"
import { NextRequest } from "next/server";

import {resolve} from "path"
import { s3_client, setStorageCommand } from "src/utils/s3_files_auth";
import {supabaseClient} from "src/utils/supabase_server"
import { Tables } from "src/utils/types_db";




export async function POST(req:NextRequest) {
    const {file, email} : {file: File, email:string} = await req.json()
    try {
    const fileBuffer = await file.arrayBuffer()
    const buffer = new Uint8Array(fileBuffer);
    var key_value = email
    const upload_command = setStorageCommand(file.name, key_value)
    const response =  await s3_client.send(upload_command)
    alert("storing the result in" + (response["$metadata"]))
    
    // fetching the latest jobId from the db
    const jobId = await supabaseClient.from("extra_surface")
    .select('job_history')
    .eq('email', email)
    .order('job_created_at', { ascending: false })
    .limit(1);
    
    supabaseClient.from("selectionjob").insert({
        job_id: jobId.data["job_history"].job_id,
        geocordinate_copc: [],
        job_created_at: new Date().getTime().toString(),
        upload_url_file: file.name,
        status: true,
    });
}
    catch(Error) {
        console.error("/api/files not working: " + Error)
    }
}