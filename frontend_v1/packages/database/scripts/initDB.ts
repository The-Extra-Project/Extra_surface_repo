import { RDSClient, CreateDBInstanceCommand } from "@aws-sdk/client-rds";
import { Client } from 'pg';
import {env} from '@repo/env';


// Configure RDS client
const rdsClient = new RDSClient({
  region: env.AWS_REGION,
  credentials: {
    accessKeyId: env.AWS_ACCESS_KEY_ID!,
    secretAccessKey: env.AWS_SECRET_ACCESS_KEY!,
  },
});

// PostgreSQL client
const dbClient = new Client({
  connectionString: env.DATABASE_URL,
});

const createTables = async () => {
  const createAdminTable = `
    CREATE TABLE IF NOT EXISTS public.admin (
      id serial PRIMARY KEY,
      email text NOT NULL UNIQUE,
      password text NOT NULL,
      created_at timestamp with time zone DEFAULT now()
    );
  `;

  const createDashboardUserTable = `
    CREATE TABLE IF NOT EXISTS public.dashboard_user (
      id serial PRIMARY KEY,
      username text NOT NULL UNIQUE,
      email text NOT NULL UNIQUE,
      created_at timestamp with time zone DEFAULT now()
    );
  `;

  const createSubscriptionTable = `
    CREATE TABLE IF NOT EXISTS realtime.subscription (
      id serial PRIMARY KEY
      -- Add other fields as necessary
    );
  `;

  const createBucketTable = `
    CREATE TABLE IF NOT EXISTS storage.bucket (
      id serial PRIMARY KEY
      -- Add other fields as necessary
    );
  `;

  const createMigrationTable = `
    CREATE TABLE IF NOT EXISTS storage.migration (
      id serial PRIMARY KEY
      -- Add other fields as necessary
    );
  `;
  try {
    await dbClient.connect();
    await dbClient.query(createAdminTable);
    await dbClient.query(createDashboardUserTable);
    await dbClient.query(createSubscriptionTable);
    await dbClient.query(createBucketTable);
    await dbClient.query(createMigrationTable);
    console.log('Tables created successfully');
  } catch (err) {
    console.error('Error creating tables', err);
  } finally {
    await dbClient.end();
  }
};

const initDB = async () => {
  try {
    const createDBInstanceCommand = new CreateDBInstanceCommand({
      DBInstanceIdentifier: process.env.DB_INSTANCE_IDENTIFIER,
      AllocatedStorage: 20,
      DBInstanceClass: "db.t2.micro",
      Engine: "postgres",
      MasterUsername: process.env.DB_MASTER_USERNAME,
      MasterUserPassword: process.env.DB_MASTER_PASSWORD,
      VpcSecurityGroupIds: [process.env.DB_SECURITY_GROUP_ID!],
    });

    await rdsClient.send(createDBInstanceCommand);
    console.log('Database instance created successfully');

    // Wait for the database to be available before creating tables
    await new Promise(resolve => setTimeout(resolve, 60000)); // Wait for 1 minute

    await createTables();
  } catch (err) {
    console.error('Error initializing database', err);
  }
};

initDB();