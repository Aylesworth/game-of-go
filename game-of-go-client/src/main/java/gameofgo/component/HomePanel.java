package gameofgo.component;

import gameofgo.common.Message;
import gameofgo.service.SocketService;
import javafx.geometry.Pos;
import javafx.scene.control.Alert;
import javafx.scene.control.Button;
import javafx.scene.control.ButtonType;
import javafx.scene.control.Label;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;

public class HomePanel extends BorderPane {
    private SocketService socketService = SocketService.getInstance();
    private OnlineBox onlineBox;

    public HomePanel() {
        onlineBox = new OnlineBox();
        setCenter(onlineBox);

        socketService.send(new Message("LSTONL", ""));
        socketService.on("LSTONL", message -> {
            onlineBox.getChildren().clear();
            for (String line : message.payload().split("\n")) {
                String[] params = line.split(" ");
                onlineBox.getChildren().add(new UserBox(params[0], params[1]));
            }
        });
        socketService.on("CHGONL",
                message -> socketService.send(new Message("LSTONL", "")));

        socketService.on("INVITE", message -> {
            String sender = message.payload().split("\n")[0];
            Alert alert = new Alert(Alert.AlertType.INFORMATION);
            alert.setHeaderText("You've got a challenge from %s. Do you want to accept?".formatted(sender));
            alert.getButtonTypes().setAll(ButtonType.YES, ButtonType.NO);
            alert.showAndWait();
        });
    }

    private class OnlineBox extends VBox {
        OnlineBox() {
            setAlignment(Pos.CENTER);
        }
    }

    private class UserBox extends HBox {
        private Label lblUsername;
        private Label lblStatus;
        private Button btnInvite;

        UserBox(String username, String status) {
            setMinWidth(300);
            setAlignment(Pos.CENTER);

            lblUsername = new Label(username);
            lblStatus = new Label(status);
            btnInvite = new Button("Invite");

            getChildren().add(lblUsername);
            if (status.equals("Available"))
                getChildren().add(btnInvite);
            getChildren().add(lblStatus);

            btnInvite.setOnMouseClicked(event ->
                    socketService.send(new Message("INVITE", username + "\n")));
        }
    }
}
