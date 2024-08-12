import { PutObjectCommand } from "@aws-sdk/client-s3";

import { Upload } from "@aws-sdk/lib-storage";
import { S3Client } from "@aws-sdk/client-s3";

import { configDotenv } from "dotenv";
import { resolve } from "path";
import { execPath } from "process";


configDotenv(
    {
        path: resolve(__dirname, '../../.env')
    }
)

export const s3_client = new S3Client(    
    {
    region: process.env.AWS_REGION,
    })
    export interface uploadS3ParamsResponse {
    location: string, 
    upload_id: string
    }

export async function uploadCommandS3(file_buffer: any, username: string): Promise<uploadS3ParamsResponse> {

    const storage_command_params  =  {
        Body: file_buffer,
        Bucket: process.env.S3_BUCKET!,
        Key: username, // required
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

const uploadToFirstS3 = (stream) => (new Promise((resolve, reject) => {
    const uploadParams = {
      Bucket: process.env.S3_BUCKET,
      Key:'some-key',
      Body: stream,
    };
    s3_client.send(new PutObjectCommand(uploadParams), (err) => {
      if (err) reject(err);
      resolve(true);
    });
  }));
  

