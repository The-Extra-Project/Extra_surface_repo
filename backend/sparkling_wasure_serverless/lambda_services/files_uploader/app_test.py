import pytest_lazyfixture
from .app  import file_uploader_handler, InputFileHandler
import dotenv import load_dotenv
from pathlib import Path
import os
load_dotenv()

def test_file_uploader_runs():
    event = InputFileHandler(username="abc@gmail.com", input_file_obj= "./test_dalle.txt")
    params =  file_uploader_handler(event=event, context=None)
    assert params["stored_path"] ==  Path(os.getenv("S3_DIR") / "test_dalle.txt").as_uri()
                                                        
       
    
