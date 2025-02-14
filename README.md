# ft_irc

Our project aims to build an IRC server that is irssi client compliant. In this server we allow multiple commands and users can message themselves. It is also possible to join the server by using `netcat`/`nc`.

### Usage

#### Server side

To start running a server, all you have to do is type:

```bash
./ircserv <port> <passoword>
```

Our standard port for testing has been `6667`. However, we greatly advise you to research on ports to understand and select the appropriate port.

### User side

If you are a user, you can either get into the server (if it is running) using `irssi` or `netcat`:

#### Using `irssi`

```bash
./irssi
```

This will lead you to `irssi` interface. Over there you will *not* be following bash syntax. Therefore, you must follow the next steps precisely to join the server:

```bash
/connect <server_ip> <port> <password>
```
