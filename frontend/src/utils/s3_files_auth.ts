import { PutObjectCommand } from "@aws-sdk/client-s3";
import { Upload, Options } from "@aws-sdk/lib-storage";
import { S3Client, PutObjectCommandInput } from "@aws-sdk/client-s3";
import { configDotenv } from "dotenv";
import { resolve } from "path";
import path from "path";
import {env} from "@/env"

configDotenv(
    {
        path: resolve(__dirname, '../../.env')
    }
)

export const s3_client = new S3Client(    
    {
    region: env.AWS_REGION,
    })
    export interface uploadS3ParamsResponse {
    location: string, 
    upload_id: string
    }

    const date_time = (new Date().toISOString())

export async function uploadCommandS3(file_buffer: any, username: string): Promise<uploadS3ParamsResponse> {
    const date_time = (new Date().getDay()).toString()
    
    const storage_command_params: PutObjectCommandInput =  {
        Body: file_buffer,
        Bucket: env.S3_BUCKET!,
        Key:  (date_time + "/" + username + "/"), // required
        ContentType: ".txt"      
    }
    try {
        const upload_s3 = new Upload({
            client: s3_client,
            params: storage_command_params
        })
    
    upload_s3.on("httpUploadProgress", (progress) => {
        console.log(progress);
      });
    
    return { location: (await upload_s3.done()).Location, upload_id: upload_s3.uploadId}
    }
    catch(err) {
        console.error("uploadCommandS3 error" + err)
        return { location: "", upload_id: ""}
    }
}