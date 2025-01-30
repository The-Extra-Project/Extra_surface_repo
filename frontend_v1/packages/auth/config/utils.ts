import {signUp, signIn, signOut, autoSignIn} from 'aws-amplify/auth'
import {SignUpParameters, SignInParameters} from './types'
import { confirmSignUp, type ConfirmSignUpInput } from 'aws-amplify/auth';

async function handleAutoSignIn() {
    try {
      const signInOutput = await autoSignIn();
      // handle sign-in steps
    } catch (error) {
      console.log(error);
    }
  }
export async function handleNormalSignUp({
    username,
    password,
    email,
    phoneNumber,

  }: SignUpParameters) {
  try {
    const {isSignUpComplete, userId, nextStep} = await signUp({
      username,
      password,
      options:{
        userAttributes:{
          email,
          phoneNumber
        },
      autoSignIn: true,
      },
    })
    console.log(userId);
     if (isSignUpComplete) {
        console.log('Sign up complete')
     }
        else {
        console.log('Sign up not complete')
        console.log(nextStep)
        }
  }
  catch (error) {
    console.error(error)
  }
  }
  


  export async function handleNormalSignIn({
    username,
    password,
  }: SignInParameters) {


    try {
      const {isSignedIn, nextStep } = await signIn({
        username,
        password,
      })
      console.log(isSignedIn)
    }
    catch (error) {
      console.error(error)
    }
  }

  export async function handleNormalSignOut() {
    try {
      await signOut();
    } catch (error) {
      console.error(error);
    }
  }


export async function handleSignUpConfirmation({
    username,
    confirmationCode
  }: ConfirmSignUpInput) {
    try {
      const { isSignUpComplete, nextStep } = await confirmSignUp({
        username,
        confirmationCode
      });
    } catch (error) {
      console.log('error confirming sign up', error);
    }
  }