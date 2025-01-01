## @extrasurface/notifications

Package that defines the functions to interact with the eventbridge for:

- sending the notification to the eventbridge connected with lambda services to instantiate the EMR cluster

- or to fetch the state of the reconstruction Job as the webhook 




## steps :

1. We require the aws cloud configurations (the `AWS_KEY_ID`, `AWS_KEY_SECRET` and also getting values of the event bridges) stored in either of the following locations:
    - in the @extrasurface/env package .env file
    - or instantiating an key-value pairs in the aws-secret-manager .  checkout the [following tutorial](https://docs.aws.amazon.com/sdk-for-javascript/v3/developer-guide/javascript_secrets-manager_code_examples.html) for setting up these values. 
