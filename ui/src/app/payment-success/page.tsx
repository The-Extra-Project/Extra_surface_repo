"use client"
import { useEffect } from "react";
import Header from "src/components/Header/Header";


export default function PaymentSuccess({
    searchParams: { amount },
    
  }: {
    searchParams: { amount: string,  email: string, URLs: string[] };
  }) {



    // useEffect(() => {
      // const sendEmail = async () => {
      //   try {
      //     const response = await fetch('/api/email', {
      //       method: 'POST',
      //       headers: {
      //         'Content-Type': 'application/json',
      //       },
      //       body: JSON.stringify({
      //         email,
      //         amount,
      //         URLs,
      //       }),
      //     });
    
      //     if (response.ok) {
      //       console.log('Email sent successfully');
      //       console.log(await response.json());
      //     } else {
      //       console.error('Error sending email:', await response.json());
      //     }
      //   } catch (error) {
      //     console.error('Error sending email:', error);
      //   }
      // };
    
    //   sendEmail();
    // }, []);
    return (
      <main className="max-w-6xl mx-auto p-10 text-black text-center border m-10 rounded-md bg-gradient-to-tr">
       <Header/>
        <div className="mb-10">
          <h1 className="text-4xl font-extrabold mb-2"> Congrats , job is scheduled and payment of  </h1>
          <div className=" text-2xl font-bold">
            ${amount} is reeived
          </div>
          <div>
        You will receive mail of confirmation along with and when you will received reconstructed data. 
          </div>
          <h1>
          </h1>
        </div>
      </main>
    );
  }