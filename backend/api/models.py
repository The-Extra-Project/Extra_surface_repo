from pydantic import BaseModel, EmailStr
from typing import Dict, List
class ScheduleJob(BaseModel):
    input_url: str
    username: EmailStr
class ScheduleJobMulti(BaseModel):
    input_url_file: str
    aggregator: int

class ReturnScheduleJob(BaseModel):
    directory: str
    index: int
    
class UserJob():
    jobId: str
    email: str
    files_storage_tiles: Dict[str,List[str]] = {}
    paymentId: str


class Status(BaseModel):
    job_status: str
