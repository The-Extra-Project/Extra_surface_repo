export { clerkMiddleware as authMiddleware } from '@clerk/nextjs/server';
import  {AuthUser} from 'aws-amplify/auth';
import {NextMiddleware, NextRequest} from 'next/server';
import {} from 'aws-amplify/adapter-core'
import {NextServer, createServerRunner}  from '@aws-amplify/adapter-nextjs';



/**
 * 
 */