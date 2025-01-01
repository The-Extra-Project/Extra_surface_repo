"use server"

import {ShortestConfig} from "@antiwork/shortest"
import { shortest } from "@antiwork/shortest"
import {env} from "@repo/env"

const getBaseUrl = (): string => {
    if(env.NODE_ENV === 'production') 
    {
      return env.NEXT_PUBLIC_API_URL;
    } 
    else(env.NODE_ENV === 'development') 
    {
      return  'http://localhost:3000';
    }   
};
  
  const getAnthropicKey = (): string | undefined => {
    return env.ANTHROPIC_API_KEY;
  };
  
  const config: ShortestConfig = {
    headless: false,
    baseUrl: getBaseUrl(),
    testDir: './__tests__',
    anthropicKey: getAnthropicKey(),
    
  };
  
  export default config;