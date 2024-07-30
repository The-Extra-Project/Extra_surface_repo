"use client"
// import { useEffect } from "react";
import Header from "src/components/Header/Header";
import { useSearchParams } from "next/navigation";
import { useState, useEffect } from "react";

export default function PaymentSuccess() {

    const getParams = useSearchParams();
    const [QueryParams, getQueryParams] = useState(0);

    useEffect(() => {
      const params = parseInt(getParams.get("amount"), 10);
      getQueryParams(params);

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

    }, [getParams]);
      return (
      <main className="max-w-6xl mx-auto p-10 text-black text-center border m-10 rounded-md bg-gradient-to-tr">
       <Header/>
        <div className="mb-10">
          <h1 className="text-4xl font-extrabold mb-2"> Congrats , job is scheduled and payment of  </h1>
          <div className=" text-2xl font-bold">
            ${QueryParams} is reeived
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