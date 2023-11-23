package gameofgo.component;

import gameofgo.common.Configs;
import gameofgo.common.Message;
import gameofgo.common.SessionStorage;
import gameofgo.service.SocketService;
import javafx.geometry.Pos;
import javafx.scene.control.*;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Priority;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.shape.Circle;

import java.util.Optional;

public class HomeView extends VBox {
    private SocketService socketService = SocketService.getInstance();
    private int selectedBoardSize;
    private VBox onlineBox;
    private ListView<HBox> onlineListView;
    private Button btnInvite;

    public HomeView() {
        Label lblPlay = new Label("PLAY");
        lblPlay.setFont(Configs.primaryFont(24));

        onlineBox = createOnlineBox();

        setAlignment(Pos.CENTER);
        setSpacing(20);
        getChildren().addAll(lblPlay, createSelectBoardSizeBox(), onlineBox);

        socketService.on("LSTONL", message -> {
            onlineListView.getItems().clear();
            btnInvite.setDisable(true);

            for (String line : message.payload().split("\n")) {
                String[] params = line.split(" ", 2);

                if (params[0].equals(SessionStorage.getItem("username")))
                    continue;

                onlineListView.getItems().add(createUserBox(params[0], params[1]));
            }
        });

        socketService.on("CHGONL",
                message -> socketService.send(new Message("LSTONL", "")));

        socketService.on("INVITE", message -> {
            String sender = message.payload().split("\n")[0];
            Alert alert = new Alert(Alert.AlertType.INFORMATION);
            alert.setHeaderText("You've got a challenge from %s. Do you want to accept?".formatted(sender));
            alert.getButtonTypes().setAll(ButtonType.YES, ButtonType.NO);
            Optional<ButtonType> choice = alert.showAndWait();

            String payload = sender + '\n';
            if (choice.isPresent() && choice.get().equals(ButtonType.YES)) {
                payload += "ACCEPT\n";
            } else {
                payload += "DECLINE\n";
            }

            socketService.send(new Message("INVRES", payload));
        });

        socketService.on("INVRES", message -> {
            String[] params = message.payload().split("\n");
            String opponent = params[0];
            String reply = params[1];

            if (reply.equals("ACCEPT")) {
                System.out.println("Player %s accepted the challenge.".formatted(opponent));
            } else {
                System.out.println("Player %s rejected the challenge.".formatted(opponent));
            }
        });

        socketService.on("SETUP", message -> {
            int color = Integer.parseInt(message.payload().substring(0, 1));
            MainWindow.getInstance().setCenter(new GameView(color));
        });

        socketService.send(new Message("LSTONL", ""));
    }

    private HBox createSelectBoardSizeBox() {
        Label lblTitle = new Label("Select board size:");
        lblTitle.setFont(Configs.primaryFont(13));

        VBox radioGroupBox = new VBox();
        radioGroupBox.setAlignment(Pos.CENTER_LEFT);
        radioGroupBox.setSpacing(5);

        ToggleGroup group = new ToggleGroup();

        int[] sizes = {9, 13, 19};
        for (int size : sizes) {
            RadioButton rbtn = new RadioButton(size + "x" + size);
            rbtn.setFont(Configs.primaryFont(12));
            rbtn.setToggleGroup(group);
            rbtn.setOnAction(event -> selectedBoardSize = size);

            radioGroupBox.getChildren().add(rbtn);

            if (size == 19) {
                rbtn.setSelected(true);
                selectedBoardSize = size;
            }
        }

        HBox selectBoardSizeBox = new HBox();
        selectBoardSizeBox.setAlignment(Pos.CENTER);
        selectBoardSizeBox.getChildren().addAll(lblTitle, radioGroupBox);
        selectBoardSizeBox.setSpacing(15);
        selectBoardSizeBox.setMaxWidth(300);

        return selectBoardSizeBox;
    }

    private VBox createOnlineBox() {
        Label lblTitle = new Label("Challenge another player:");
        lblTitle.setFont(Configs.primaryFont(13));

        onlineListView = new ListView<>();
        onlineListView.setMaxHeight(250);
        onlineListView.setOnMouseClicked(event -> {
            HBox selectedItem = onlineListView.getSelectionModel().getSelectedItem();
            if (selectedItem == null) {
                btnInvite.setDisable(true);
                return;
            }

            if (((Label) ((HBox) selectedItem.getChildren().get(1)).getChildren().get(1)).getText().equals("Available")) {
                btnInvite.setDisable(false);
            } else {
                btnInvite.setDisable(true);
            }
        });

        btnInvite = new Button("Invite");
        btnInvite.setOnAction(event -> {
            HBox selectedItem = onlineListView.getSelectionModel().getSelectedItem();
            String username = ((Label) ((HBox) selectedItem.getChildren().get(0)).getChildren().get(0)).getText();
            socketService.send(new Message("INVITE", username + "\n"));
        });

        VBox onlineBox = new VBox();
        onlineBox.setAlignment(Pos.CENTER);
        onlineBox.setMaxWidth(300);
        onlineBox.setSpacing(10);
        onlineBox.getChildren().addAll(lblTitle, onlineListView, btnInvite);

        return onlineBox;
    }

    private HBox createUserBox(String username, String status) {
        Label lblUsername = new Label(username);
        Label lblStatus = new Label(status);
        lblStatus.setMinWidth(30);

        double circleSize = 5;
        Circle circleStatus = new Circle(circleSize, circleSize, circleSize);
        circleStatus.setFill(status.equals("Available") ? Color.LIGHTGREEN : Color.RED);

        HBox usernameBox = new HBox(lblUsername);
        usernameBox.setAlignment(Pos.CENTER_LEFT);
        HBox.setHgrow(usernameBox, Priority.ALWAYS);

        HBox statusBox = new HBox(circleStatus, lblStatus);
        statusBox.setAlignment(Pos.CENTER_RIGHT);
        statusBox.setSpacing(5);
        HBox.setHgrow(statusBox, Priority.ALWAYS);

        HBox userBox = new HBox();
        userBox.setMaxWidth(280);
        userBox.setAlignment(Pos.CENTER);
        userBox.getChildren().addAll(usernameBox, statusBox);

        return userBox;
    }
}

