## Hosting the Server on AWS

Follow these steps to host the server on an AWS EC2 instance and run the client on your local machine.

### Prerequisites

- AWS account
- EC2 instance (Ubuntu or any preferred Linux distribution)
- SSH access to your EC2 instance
- Public IP address of your EC2 instance

### Steps to Host the Server on AWS

1. **Launch an EC2 Instance**:
   - Log in to your AWS Management Console.
   - Navigate to the EC2 Dashboard and click "Launch Instance".
   - Select an Amazon Machine Image, Ubuntu.
   - Choose an instance type (e.g., t3.micro for free tier).
   - Configure the instance details as needed.
   - Add storage if necessary.
   - Configure the security group:
     - Add a rule to allow inbound traffic on port 8080 for TCP.
     - Ensure you also have an SSH rule allowing inbound traffic on port 22.
      
2. **Connect to the EC2 Instance**:
   - Open your terminal 
   - vi server.c
   - Paste client side code

3. **Install Necessary Packages on the EC2 Instance**:
   - Update the package lists and install GCC and other necessary tools:
     ```sh
     sudo apt-get update
     sudo apt install gcc
     ```

4. **Compile the Server Code**:

     ```sh
     cc -o server server.c
     ```

6. **Run the Server**:
   - On your EC2 instance, start the server:
     ```sh
     ./server
     ```

### Updating the Client Code

1. **Replace the IP Address**:
   - Open the `client.c` file on your local machine.
   - Locate the line where the IP address is set:
     ```c
     if (inet_pton(AF_INET, "ip_address", &serv_addr.sin_addr) <= 0) {
     ```
   - Replace `"ip_address"` with the public IP address of your EC2 instance:
     ```c
     if (inet_pton(AF_INET, "your-ec2-public-ip", &serv_addr.sin_addr) <= 0) {
     ```

### run both the server and client on your local machine or on the same local network, follow these steps:

1. **Update the IP Address**:
   - In the `client.c` file, replace the server IP address with the local network IP address of the machine running the server or use `localhost` if both are on the same machine:
     ```c
     if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
     ```
### Running the Client

1. **Compile and Run the Client**:
   - Compile the client code on your local machine:
     ```sh
     cc -o client client.c
     ```
   - Run the client:
     ```sh
     ./client
     ```

2. **Interact with the Server**:
   - Use the client application to interact with the server. You can sign up, log in, add contacts, send messages, and view received messages.

By following these steps, you should be able to successfully host your server on an AWS EC2 instance and connect to it using your client application.
