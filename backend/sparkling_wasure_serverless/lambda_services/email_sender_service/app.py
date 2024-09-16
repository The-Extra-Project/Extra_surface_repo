import resend
import os
from dotenv import load_dotenv
from pathlib import Path
from pydantic import EmailStr, BaseModel, StrictStr
from aws_lambda_powertools.logging import Logger
env_dir = Path(__file__) / '.env'
load_dotenv(dotenv_path=env_dir)
resend.api_key = os.getenv("RESEND_API_KEY")

logger_obj = Logger()

class EmailHandlerInputs(BaseModel):
    email_category: StrictStr
    details: list

class EmailParams():
    sender: EmailStr = "support@extralabs.xyz"
    destination: EmailStr 
    subject: str
    mail_content: str
    reply_to: EmailStr
    cc: str = ""

sujet_map_en = {
    "payment_completed": ["payment acknowledged! queued the job"],
    "job_scheduled": ["The job is queued"],
    "job_result_generated": ["the result is generated"],
    "job_status_update": ["result taking more time than anticipated"]
}

contenu_mail_map_en = {
    "payment_completed": ["<Strong> Thank you for completing the payment_id: </Strong>", "<a2> We will be later commencing the job execution of lidar tiles once the compute infrastructure is available and will share soon the url of the results</a2>", "Just to take care that the jobs take atleast 3 to 48 hrs of processing (and due to numerous jobs) to create the finished tiles."], 
    "job_scheduled": ["<Strong> Your Job Number</Strong>", " Is Scheduled with the cluster <br/>. you can do the job progress via the url:"],
    "job_result_generated": ["Its the time: thanks for the paitence, your reconstruction job scheduled on: ", "is completed and the data is stored in the IPFS storage here:" , "kindly download the 3DTiles files and then use itowns in order to visualize the results"],
    "job_status_update": ["Hi there, i wanted to update that due to issues of compute bandwidth constraints ,your reconstruction job is taking more time than anticipated, so kindly wait couple of days or contact Charlie (Charlie@extralabs.fr for more details)"]
}


sujet_map = {
  "paiement_effectue": ["Paiement confirmé ! Traitement en attente pour maillage"],
  "travail_planifie": ["Le traitement est commence"],
  "resultat_travail_genere": ["Le résultat est généré"],
  "mise_a_jour_statut_travail": ["Le résultat prend plus de temps que prévu"]
}

contenu_mail_map = {
  "paiement_effectue": ["<Strong> Merci d'avoir effectué le paiement_id : </Strong>", "<a2> Nous commencerons plus tard l'exécution du traitement des tuiles lidar une fois que l'infrastructure de calcul sera disponible et nous partagerons bientôt l'URL des résultats</a2>", "Veuillez noter que la création des tuiles finales nécessite un traitement de 3 à 48 heures minimum (en raison du nombre important de traitements en cours)."],
  "travail_planifie": ["<Strong> Votre numéro de traitement </Strong>", "Est planifié avec le cluster <br/>. Vous pouvez suivre la progression du traitement via l'URL :"],
  "resultat_travail_genere": ["C'est le moment : merci pour votre patience, votre traitement de reconstruction programmé le : ", "est terminé et les données sont stockées dans le stockage IPFS ici : ", "Veuillez télécharger les fichiers 3DTiles et ensuite utiliser itowns pour visualiser les résultats"],
  "mise_a_jour_statut_travail": ["Bonjour, je voulais vous informer qu'en raison de problèmes de bande passante de calcul, votre traitement de reconstruction prend plus de temps que prévu. Veuillez donc patienter quelques jours ou contacter Charlie (Charlie@extralabs.fr) pour plus de détails."]
}

def send_payment_notification(receiver_email, payment_intent_id, file_details ):
    envoyer_email_notif = f"""
    Merci d'avoir passé commande de tuiles 3D depuis notre infrastructure. <br />
    Le traitement de vos données va bientôt commencer. Vous recevrez les résultats par mail dès qu'ils seront disponibles (compter jusqu'à 36h de délai). <br />
    <h3>Montant du paiement: <h3/> {payment_intent_id} <br />
    <h3>Nom du fichier reconstruits: <h3/> {file_details}
    """

    try:
        params = EmailParams()
        params.destination = receiver_email
        params.mail_content = envoyer_email_notif
        params.subject = "Paiement confirmé ! le traitement des données va commencer"
        params.cc = "hello@extralabs.xyz"       
       
        params_email : resend.Emails.SendParams  = {
            "from": params.sender,
            "to": params.destination,
            "subject": params.subject,
            "html": params.mail_content,
            "cc": params.cc
        }
        
        emailObj = resend.Emails.send(
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
        params.mail_content = contenu_mail_map["job_scheduled"][0] + job_id + "\n" + contenu_mail_map["job_scheduled"][1] + contenu_mail_map["job_scheduled"][2]  
        params.subject = sujet_map["job_scheduled"][0]
        params.cc = str("hi@extralabs.xyz")       
        params_email : resend.Emails.SendParams  = {
            "from": params.sender,
            "to": params.destination,
            "subject": params.subject,
            "html": params.mail_content,
            "cc": params.cc
        }
        emailObj = resend.Emails.send(
            params=params_email
        )
        print(f"Sent email for job reconstruction")
        print("Email ID: ", emailObj["id"])
        return emailObj["id"]

    except Exception as e:
        print("unable to send the reconstruction job mail due to "+ str(e))


def send_notification(receiver_email):
    try:
        params = EmailParams()
        params.destination = receiver_email
        params.mail_content = contenu_mail_map["job_status_update"][0]  + "\n" 
        params.subject = sujet_map["job_status_update"][0]
        params.cc = str("hi@extralabs.xyz")            
        params_email : resend.Emails.SendParams  = {
            "from": params.sender,
            "to": params.destination,
            "subject": params.subject,
            "html": params.mail_content,
            "cc": params.cc
        }
        emailObj = resend.Emails.send(
            params=params_email
        )
        print(f"Sent email for job reconstruction")
        print("Email ID: ", emailObj["id"])
        return emailObj["id"]

    except Exception as e:
        print("unable to send the reconstruction job mail due to "+ str(e))
def send_job_results(receiver_email, job_id , ipfs_url):
    try:
        params = EmailParams()
        params.destination = receiver_email
        params.mail_content = contenu_mail_map["job_scheduled"][0] + contenu_mail_map["job_scheduled"][1] + job_id + "\n" + contenu_mail_map["job_scheduled"][2] + contenu_mail_map["job_scheduled"][2] + ipfs_url +  "\n" + contenu_mail_map["job_scheduled"][3]
        params.subject = sujet_map["job_scheduled"][0]
        params.cc = str("hi@extralabs.xyz")             
        params_email : resend.Emails.SendParams  = {
            "from": params.sender,
            "to": params.destination,
            "subject": params.subject,
            "html": params.mail_content,
            "cc": params.cc
        }
        emailObj = resend.Emails.send(
            params=params_email
        )
        print(f"Sent email for job reconstruction")
        print("Email ID: ", emailObj["id"])
        return emailObj["id"]

    except Exception as e:
        print("unable to send the job results mail due to "+ str(e))

def email_sending_serverless(event: EmailHandlerInputs, context):
    try:
        input_params = event.model_dump()["details"]
        if event.model_dump()["email_category"] == "send_payment_notification" and input_params:
            send_payment_notification(receiver_email=input_params[0], payment_intent_id= input_params[1], file_details= input_params[2])
            logger_obj.debug("send the deployment notification")
        if event.model_dump()["email_category"] == "send_job_reconstruction" and input_params:
            send_job_reconstruction(receiver_email=input_params[0],job_id=input_params[1])
        elif event.model_dump()["email_category"] == "send_job_results" and input_params:
            send_job_results(receiver_email=input_params[0], job_id= input_params[1], ipfs_url= input_params[2])    
    except Exception as e:
        print("demo")