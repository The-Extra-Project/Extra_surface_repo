import 'server-only';
import { signIn, signOut, AuthUser } from 'aws-amplify/auth';

export type DeletedObjectJSON = {
  id: string;
};

export type OrganizationJSON = {
  id: string;
  name: string;
  image_url?: string;
  created_by: string;
};

export type OrganizationMembershipJSON = {
  organization: OrganizationJSON;
  public_user_data: {
    user_id: string;
  };
};

export type UserJSON = {
  id: string;
  email_addresses: { email_address: string }[];
  first_name?: string;
  last_name?: string;
  created_at: string;
  image_url?: string;
  phone_numbers: { phone_number: string }[];
};

export type WebhookEvent = {
  type: string;
  data: any;
};


export * from '@aws-amplify/adapter-nextjs'
export * from 'aws-amplify'