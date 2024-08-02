import resend
import os
from dotenv import load_dotenv
from pathlib import Path
from pydantic import EmailStr
#import resend.emails
env_dir = Path(__file__).parent.parent.parent / '.env'
load_dotenv(dotenv_path=env_dir)
resend.api_key = os.environ.get('RESEND_API_KEY')

class EmailParams():
    sender: EmailStr = "charlie@extralabs.xyz"
    destination: EmailStr 
    subject: str
    mail_content: str
    reply_to: EmailStr
    cc: str



subject_map = {
    "payment_completed": ["payment acknowledged! queued the job"],
    "job_scheduled": ["The job is queued"],
    "job_result_generated": ["the result is generated"],
    "job_status_update": ["result taking more time than anticipated"]
}

mail_content_map = {
    "payment_completed": ["<Strong> Thank you for completing the payment_id: </Strong>", "<a2> We will be later commencing the job execution of lidar tiles once the compute infrastructure is available and will share soon the url for you to monitor the job</a2>"], 
    "job_scheduled": ["<Strong> Your Job Number</Strong>", " Is Scheduled with the cluster <br/>. you can do the job progress via the url:"],
    "job_result_generated": ["Its the time: thanks for the paitence, your reconstruction job scheduled on: ", "is completed and the data is stored in the IPFS storage here:" , "kindly download the 3DTiles files and then use itowns in order to visualize the results"],
    "job_status_update": ["Hi there, i wanted to update that due to issues of compute bandwidth constraints ,your reconstruction job is taking more time than anticipated, so kindly wait couple of days or contact Charlie (Charlie@extralabs.fr for more details)"]
}

def send_payment_notification(receiver_email, payment_intent_id ):
    try:
        params = EmailParams()
        params.destination = receiver_email
        params.mail_content = mail_content_map["payment_completed"][0] + payment_intent_id + "\n" + mail_content_map["payment_completed"][1]
        params.subject = subject_map["payment_completed"][0]
        params.cc = str(os.environ.get("SUPERADMIN_EMAIL_ADDRESS"))       
        
        params_email : resend.Emails.SendParams  = {
            "from": params.sender,
            "to": params.destination,
            "subject": params.subject,
            "html": params.mail_content,
            "cc": params.cc
        }
        
        emailObj: resend.Email = resend.Emails.send(
            params=params_email
        )
        print(f"Sent email for payment notification")
        print("Email ID: ", emailObj["id"])
        return emailObj

    except Exception as e:
        print("unable to send the payment notification due to "+ str(e))


def send_job_reconstruction(receiver_email, job_id):
    try:
        params = EmailParams()
        params.destination = receiver_email
        params.mail_content = mail_content_map["job_scheduled"][0] + job_id + "\n" + mail_content_map["job_scheduled"][1] + mail_content_map["job_scheduled"][2] + cluster_dashbaord_url 
        params.subject = subject_map["job_scheduled"][0]
        params.cc = str(os.environ.get("SUPERADMIN_EMAIL_ADDRESS"))       
        params_email : resend.Emails.SendParams  = {
            "from": params.sender,
            "to": params.destination,
            "subject": params.subject,
            "html": params.mail_content,
            "cc": params.cc
        }
        emailObj = resend.Email = resend.Emails.send(
            params=params_email
        )
        print(f"Sent email for job reconstruction")
        print("Email ID: ", emailObj["id"])
        return emailObj

    except Exception as e:
        print("unable to send the reconstruction job mail due to "+ str(e))


def send_notification(receiver_email):
    try:
        params = EmailParams()
        params.destination = receiver_email
        params.mail_content = mail_content_map["job_status_update"][0]  + "\n" 
        params.subject = subject_map["job_status_update"][0]
        params.cc = str(os.environ.get("SUPERADMIN_EMAIL_ADDRESS"))       
        params_email : resend.Emails.SendParams  = {
            "from": params.sender,
            "to": params.destination,
            "subject": params.subject,
            "html": params.mail_content,
            "cc": params.cc
        }
        emailObj = resend.Email = resend.Emails.send(
            params=params_email
        )
        print(f"Sent email for job reconstruction")
        print("Email ID: ", emailObj["id"])
        return emailObj

    except Exception as e:
        print("unable to send the reconstruction job mail due to "+ str(e))



def send_job_results(receiver_email, job_id , ipfs_url):
    try:
        params = EmailParams()
        params.destination = receiver_email
        params.mail_content = mail_content_map["job_scheduled"][0] + mail_content_map["job_scheduled"][1] + job_id + "\n" + mail_content_map["job_scheduled"][1] + mail_content_map["job_scheduled"][2] + ipfs_url +  "\n" + mail_content_map["job_scheduled"][3]
        params.subject = subject_map["job_scheduled"][0]
        params.cc = str(os.environ.get("SUPERADMIN_EMAIL_ADDRESS"))       
        params_email : resend.Emails.SendParams  = {
            "from": params.sender,
            "to": params.destination,
            "subject": params.subject,
            "html": params.mail_content,
            "cc": params.cc
        }
        emailObj = resend.Email = resend.Emails.send(
            params=params_email
        )
        print(f"Sent email for job reconstruction")
        print("Email ID: ", emailObj["id"])
        return emailObj

    except Exception as e:
        print("unable to send the job results mail due to "+ str(e))
    