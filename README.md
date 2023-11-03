# Origins

Origins is an Unreal Engine 5 demo project representing a playable short created by the team at Inworld AI to showcase NPCs powered by artificial intelligence.

## Inworld Authentication
To authorize your requests, you'll need a base64 authorization signature associated with your Inworld account.

Follow these steps to retrieve them:
- Log into Inworld Studio https://studio.inworld.ai/
- Go to the Integrations tab
- Under API Keys, generate a new key pair.
- Click the "Copy Base64" button next to your API key to copy the authorization signature to your clipboard.

Once copied, open the map ML_Setup and run the game. Enter the key from within the menu shown.

## Prerequisites
- Visual Studio (https://docs.inworld.ai/docs/tutorial-integrations/unreal-engine/getting-started/#installing-visual-studio)
- Unreal Engine 5.1

## Getting started
- Clone the repository
- Double click `InWorldRT.uproject`
- Agree to rebuild missing modules

In case of `could not be compiled` error
- Right click `InWorldRT.uproject`, select `Generate Visual Studio project files`
- After `.sln` file generated open Visual Studio soulution and build the project
- Run the project
