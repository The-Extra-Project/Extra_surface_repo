from pydantic import BaseModel, EmailStr
from typing import Dict, List


## for the supabase client.
class ScheduleJob(BaseModel):
    input_url: str
    username: EmailStr
    
class JobRuntime(BaseModel):
    jobId: str
    container_id: str

class ScheduleJobMulti(BaseModel):
    input_url_file: str
    aggregator: int
    username: EmailStr

class ReturnScheduleJob(BaseModel):
    directory: str
    index: int
    
class UserJob():
    jobId: str
    email: str
    files_storage_tiles: Dict[str,List[str]] = {}
    paymentId: str

class JobHistory(BaseModel):
    jobId: str
    status: bool
    upload_url_file: str
    
class Status(BaseModel):
    job_status: str
    
## defined for SNS (TODO: need to cleanup and make standardise with the supabase).
class PushResultReconnstruction():
    jobId: str
    username: str
    fileDir: str