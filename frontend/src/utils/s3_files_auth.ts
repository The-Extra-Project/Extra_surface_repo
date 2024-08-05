import * as AWS from "@aws-sdk/client-s3"
import { PutObjectCommand } from "@aws-sdk/client-s3";

export const s3_client = new AWS.S3Client({
    region: process.env.AWS_REGION
})

export function setStorageCommand(filename: string, username: string): PutObjectCommand {
    const storage_command  =  new PutObjectCommand({
        Body: filename,
        Bucket: process.env.S3_DIR!,
        Key: username, // required
    })
    return storage_command
}



