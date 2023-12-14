package gameofgo.service;

import gameofgo.common.Message;
import javafx.application.Platform;
import javafx.concurrent.Task;
import javafx.scene.control.Alert;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.InputStreamReader;
import java.net.NoRouteToHostException;
import java.net.Socket;
import java.util.HashMap;
import java.util.Map;

public class SocketService {
    private static SocketService instance;

    public static SocketService getInstance() {
        if (instance == null)
            instance = new SocketService();

        return instance;
    }

    private static final String SERVER_ADDRESS = "192.168.43.4";
    private static final int SERVER_PORT = 8080;
    private static final int BUFFER_SIZE = 2048;
    private Socket socket;
    private DataInputStream in;
    private DataOutputStream out;
    private BufferedReader br;
    private Map<String, MessageHandler> handlers;
    private Thread listeningThread;
    private String remainingBytes = "";

    private SocketService() {
        try {
            socket = new Socket(SERVER_ADDRESS, SERVER_PORT);
            in = new DataInputStream(socket.getInputStream());
            out = new DataOutputStream(socket.getOutputStream());
            br = new BufferedReader(new InputStreamReader(System.in));

            handlers = new HashMap<>();

            listen();
        } catch (NoRouteToHostException e) {
            Alert alert = new Alert(Alert.AlertType.ERROR);
            alert.setHeaderText("Cannot connect to server!");
            alert.showAndWait();
            System.exit(-1);
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
        String buff;

        int blockTypeLength = 1;
        int headerLength = messageType.length() + blockTypeLength + 2;
        while (headerLength + payload.length() > BUFFER_SIZE - 1) {
            buff = messageType + " M " + (BUFFER_SIZE - 1 - headerLength) + "\n" + payload.substring(0, BUFFER_SIZE - 1 - headerLength);
            send(buff);
            payload = payload.substring(BUFFER_SIZE - 1 - headerLength);
            System.out.println("Sent:\n" + buff);
        }

        buff = messageType + " L " + payload.length() + "\n" + payload;
        send(buff);
        System.out.println("Sent:\n" + buff);
    }

    public void send(Message message, long intervalBeforeMillis, long intervalAfterMillis) {
        try {
            Thread.sleep(intervalBeforeMillis);
            send(message);
            Thread.sleep(intervalAfterMillis);
        } catch (InterruptedException e) {
            throw new RuntimeException(e);
        }
    }

    public Message receive() {
        try {
            String messageType, blockType;
            int payloadLength;
            StringBuilder payloadBuilder = new StringBuilder();
            byte[] bytes = new byte[BUFFER_SIZE];
            String buff;

            do {
                int bytesRead = in.read(bytes);
                buff = new String(bytes, 0, bytesRead, "UTF-8");
                System.out.println("Received:\n" + buff);

                buff = remainingBytes + buff;

                String[] contents = buff.split("\n", 2);
                String[] header = contents[0].split(" ");
                String payload = contents[1];

                messageType = header[0];
                blockType = header[1];
                payloadLength = Integer.parseInt(header[2]);

                if (payloadLength < payload.length()) {
                    remainingBytes = payload.substring(payloadLength);
                    payload = payload.substring(0, payloadLength);
                } else {
                    remainingBytes = "";
                }

                payloadBuilder.append(payload);
            } while (!blockType.equals("L"));

            return new Message(messageType, payloadBuilder.toString());
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public void on(String messageType, MessageHandler handler) {
        handlers.put(messageType, handler);
    }

    public void removeListeners(String... messageTypes) {
        for (String messageType : messageTypes) {
            handlers.remove(messageType);
        }
    }

    private void listen() {
        listeningThread = new Thread(() -> {
            while (true) {
                try {
                    socket.setSoTimeout(500);
                    Message receivedMessage = receive();
                    String messageType = receivedMessage.messageType();
                    if (handlers.containsKey(messageType)) {
                        Platform.runLater(() -> {
                            handlers.get(messageType).handle(receivedMessage);
                        });
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
