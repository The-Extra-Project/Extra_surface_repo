from importlib import metadata
from pathlib import Path
from aws_lambda_powertools.logging.logger import Logger
from datetime import datetime
from s3path import S3Path
from mypy_boto3_s3 import S3Client
import boto3 
import requests
import os
import xml
from typing import AnyStr
from smart_open import open
from pydantic import BaseModel
import requests
class InputFileHandler(BaseModel):
    input_file_obj: open
    username: str

logger = Logger()
s3_obj: S3Client = boto3.Session().client("s3")

def file_uploader_handler(event:  InputFileHandler, context) -> str:
    """
    uploads the passed url file.
    event: its the event generated describing the metadata of the given url file.
    """
    logger.debug({
        "event file open executed with reference input": event
    })
    fileobj = str(event.model_dump()['input_path'])
    key_path = event.model_dump()['username']
    try:
        for  line in fileobj:
            urlpath = line            
            file_url = requests.get(url=urlpath, stream=True)
            s3_obj.upload_fileobj(file_url.raw,str(os.getenv("S3_BUCKET")) + "/" + key_path, key_path)
            ## now fetching the details of the file_upload
            objects = s3_obj.list_objects(Bucket=str(os.getenv("S3_BUCKET")))
            # Find the last uploaded file by sorting the objects based on the LastModified timestamp
            last_uploaded_file = sorted(objects, key=lambda x: x["LastModified"], reverse=True)[0]
            logger.debug(msg="uploaded the url file for user: " + key_path + " as :" + last_uploaded_file.name )
            last_uploaded_file.close()
            
           
        stored_path = helper_upload_files(last_uploaded_file.name,key_path)
        logger.debug(msg=" downloaded all the laz files along with : " + key_path + " as :" + stored_path)
        return str(stored_path)
    except Exception as e:
        print("file uploader service failed due to:" + str(e)) 
    return "Error File uploader: Unvalid path"

def helper_upload_files(filepath: S3Path, username: str):
    """
    uploads each of the laz tile into the corresponding folder 
    """
    try:
        with filepath.open() as f:
            for urlpath in f:
                file_url = requests.get(url=urlpath.strip(), stream=True)
                s3_key = f"{os.getenv('S3_DIR')}/{username}/{filepath.name}/{urlpath.strip(":")[0]}"
                s3_obj.upload_fileobj(file_url.raw, s3_key, username)
                
                xml_file = helper_upload_xml(s3_key)
                logger.debug(msg="uploaded the laz file: " + s3_key + " and xml file : " + str(xml_file) )
                ## returning the names of folder 
                return s3_key
    except Exception as e:
        print(f"An error occured:{e}")        

    
def helper_upload_xml(filepath):
    """
    Creates the folder with the laz file along with the xml file which is required for the docker container as parameter.   
    """
    metadata_xml = """
    <?xml version="1.0"?>
    <env>
    <datasets>
        <austin>
        <dim>3</dim>
        <ndtree_depth>1</ndtree_depth>
        <max_ppt>50000</max_ppt>
        <mode>1</mode>
        </austin>
    </datasets>
    </env>
    """
    
    try:
        s3_path = S3Path(filepath)
        
        with s3_path.open() as f:
            f.write(metadata_xml)
            f.close()
        return s3_path.as_uri()
    except Exception as e:
        print("not able to upload xml file to laz")    