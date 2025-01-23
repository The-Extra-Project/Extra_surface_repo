import { NextApiRequest, NextApiResponse } from 'next';
import { resend } from '@repo/email';
import { env } from '@repo/env';

const subjectMap = {
  payment_completed: "Payment acknowledged! Queued the job",
  job_scheduled: "The job is queued",
  job_result_generated: "The result is generated",
  job_status_update: "Result taking more time than anticipated"
};

const mailContentMap = {
  payment_completed: `
    <strong>Thank you for completing the payment_id:</strong>
    <p>We will be later commencing the job execution of lidar tiles once the compute infrastructure is available and will share soon the URL for you to monitor the job.</p>
    <p>Just to take care that the jobs take at least 3 to 48 hrs of processing (and due to numerous jobs) to create the finished tiles.</p>
  `,
  job_scheduled: `
    <strong>Your Job Number</strong>
    <p>Is Scheduled with the cluster. You can do the job progress via the URL:</p>
  `,
  job_result_generated: `
    <p>It's the time: thanks for the patience, your reconstruction job scheduled on:</p>
    <p>is completed and the data is stored in the IPFS storage here:</p>
    <p>Kindly download the 3DTiles files and then use itowns in order to visualize the results.</p>
  `,
  job_status_update: `
    <p>Hi there, I wanted to update that due to issues of compute bandwidth constraints, your reconstruction job is taking more time than anticipated, so kindly wait a couple of days or contact Charlie (Charlie@extralabs.fr for more details).</p>
  `
};

interface EmailParams {
    sender: string;
    destination: string;
    subject: string;
    mail_content: string;
    cc?: string;
}
const sendEmail = async (params:EmailParams) => {
  try {
    const emailParams = {
      from: params.sender,
      to: params.destination,
      subject: params.subject,
      html: params.mail_content,
      cc: params.cc
    };

    const emailObj = await resend.emails.send(emailParams);
    console.log(`Sent email: ${params.subject}`);
    console.log("Email ID: ", emailObj.data?.id);
    return emailObj;
  } catch (error) {
    console.error("Unable to send email due to: ", error);
  }
};

const handler = async (req: NextApiRequest, res: NextApiResponse) => {
  const { type, receiver_email, payment_intent_id, job_id, ipfs_url } = req.body;

  let params = {
    sender: "charlie@extralabs.xyz",
    destination: receiver_email,
    subject: "",
    mail_content: "",
    cc: env.SUPERADMIN_EMAIL_ADDRESS
  };

  switch (type) {
    case "payment_completed":
      params.subject = subjectMap.payment_completed;
      params.mail_content = mailContentMap.payment_completed + payment_intent_id;
      break;
    case "job_scheduled":
      params.subject = subjectMap.job_scheduled;
      params.mail_content = mailContentMap.job_scheduled + job_id;
      break;
    case "job_result_generated":
      params.subject = subjectMap.job_result_generated;
      params.mail_content = mailContentMap.job_result_generated + ipfs_url;
      break;
    case "job_status_update":
      params.subject = subjectMap.job_status_update;
      params.mail_content = mailContentMap.job_status_update;
      break;
    default:
      return res.status(400).json({ error: "Invalid email type" });
  }

  const emailObj = await sendEmail(params);
  if (emailObj) {
    return res.status(200).json({ message: "Email sent successfully", emailId: emailObj.data?.id });
  } else {
    return res.status(500).json({ error: "Failed to send email" });
  }
};

export default handler;