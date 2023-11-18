package gameofgo.service;

import gameofgo.common.Message;
import javafx.application.Platform;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.InputStreamReader;
import java.net.Socket;
import java.util.HashMap;
import java.util.Map;

public class SocketService {
    private static SocketService INSTANCE;

    public static SocketService getInstance() {
        if (INSTANCE == null)
            INSTANCE = new SocketService();

        return INSTANCE;
    }

    private static final String SERVER_ADDRESS = "localhost";
    private static final int SERVER_PORT = 8080;
    private static final int BUFFER_SIZE = 1024;
    private Socket socket;
    private DataInputStream in;
    private DataOutputStream out;
    private BufferedReader br;
    private Map<String, MessageHandler> handlers;
    private Thread listeningThread;

    private SocketService() {
        try {
            socket = new Socket(SERVER_ADDRESS, SERVER_PORT);
            in = new DataInputStream(socket.getInputStream());
            out = new DataOutputStream(socket.getOutputStream());
            br = new BufferedReader(new InputStreamReader(System.in));

            handlers = new HashMap<>();

            listen();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    private void send(String buffer) {
        try {
            byte[] bytes = buffer.getBytes("UTF-8");
            out.write(bytes);
            out.flush();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public void send(Message message) {
        String messageType = message.messageType();
        String payload = message.payload();

        System.out.println("Sent:\n" + payload);

        int blockTypeLength = 4;
        int headerLength = messageType.length() + blockTypeLength + 2;
        while (headerLength + payload.length() > BUFFER_SIZE - 1) {
            send(messageType + " MID \n" + payload.substring(0, BUFFER_SIZE - 1 - headerLength));
            payload = payload.substring(BUFFER_SIZE - 1 - headerLength);
        }
        send(messageType + " LAST\n" + payload);
    }

    public Message receive() {
        try {
            String messageType, blockType;
            StringBuilder payloadBuilder = new StringBuilder();
            byte[] bytes = new byte[BUFFER_SIZE];

            do {
                int bytesRead = in.read(bytes);
                String response = new String(bytes, 0, bytesRead, "UTF-8");

                String[] contents = response.split("\n", 2);
                String[] header = contents[0].split(" ", 2);
                messageType = header[0];
                blockType = header[1];
                payloadBuilder.append(contents[1]);
            } while (!blockType.equals("LAST"));

            System.out.println("Received:\n" + payloadBuilder);
            return new Message(messageType, payloadBuilder.toString());
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public void on(String messageType, MessageHandler handler) {
        handlers.put(messageType, handler);
    }

    private void listen() {
        listeningThread = new Thread(() -> {
            while (true) {
                try {
                    socket.setSoTimeout(500);
                    Message receivedMessage = receive();
                    String messageType = receivedMessage.messageType();
                    if (handlers.containsKey(messageType)) {
                        Platform.runLater(() ->
                                handlers.get(messageType).handle(receivedMessage));
                    }
                } catch (Exception e) {
                    if (Thread.currentThread().isInterrupted())
                        break;
                }
            }
        });
        listeningThread.start();
    }

    public void close() {
        try {
            listeningThread.interrupt();
            listeningThread.join();

            socket.close();
            in.close();
            out.close();
            br.close();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public static interface MessageHandler {
        void handle(Message message);
    }
}
