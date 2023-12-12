package gameofgo.component;

import gameofgo.common.Configs;
import gameofgo.common.Message;
import gameofgo.common.SessionStorage;
import gameofgo.service.SocketService;
import javafx.collections.FXCollections;
import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.control.*;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Priority;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.shape.Circle;

import java.util.Optional;

public class PlayTab extends Tab {
    private final SocketService socketService = SocketService.getInstance();
    private int selectedBoardSize;
    private ListView<HBox> onlineListView;
    private Button btnInvite;

    public PlayTab() {
        super("Play");
        this.setContent(createContent());
    }

    private Node createContent() {
        Label lblPlay = new Label("PLAY");
        lblPlay.setFont(Configs.primaryFont(24));

        VBox playVBox = new VBox();
        playVBox.setAlignment(Pos.CENTER);
        playVBox.setSpacing(20);
        playVBox.getChildren().addAll(lblPlay, createSelectBoardSizeBox(), createOnlineBox());

        socketService.on("LSTONL", message -> {
            onlineListView.getItems().clear();
            btnInvite.setDisable(true);

            for (String line : message.payload().split("\n")) {
                String[] params = line.split(" ", 2);

                if (params.length != 2) return;

                if (params[0].equals(SessionStorage.getItem("username")))
                    continue;

                onlineListView.getItems().add(createUserBox(params[0], params[1]));
            }
        });

        socketService.on("CHGONL",
                message -> socketService.send(new Message("LSTONL", "")));

        socketService.on("INVITE", message -> {
            String[] params = message.payload().split("\n");
            String sender = params[0];
            int boardSize = Integer.parseInt(params[1]);

            Alert alert = new Alert(Alert.AlertType.INFORMATION);
            alert.setHeaderText("You've got a challenge from %s. Do you want to accept?".formatted(sender));
            alert.getButtonTypes().setAll(ButtonType.YES, ButtonType.NO);
            Optional<ButtonType> choice = alert.showAndWait();

            String payload = "%s\n%d\n%s\n".formatted(
                    sender,
                    boardSize,
                    choice.isPresent() && choice.get().equals(ButtonType.YES) ? "ACCEPT" : "DECLINE"
            );
            socketService.send(new Message("INVRES", payload));
        });

        socketService.on("INVRES", message -> {
            String[] params = message.payload().split("\n");
            String opponent = params[0];
            String reply = params[2];

            if (reply.equals("ACCEPT")) {
                System.out.println("Player %s accepted the challenge.".formatted(opponent));
            } else {
                System.out.println("Player %s rejected the challenge.".formatted(opponent));
            }
        });

        socketService.on("SETUP", message -> {
            socketService.removeListeners("LSTONL", "CHGONL", "INVITE", "INVRES", "SETUP");
            String[] params = message.payload().split("\n");
            int boardSize = Integer.parseInt(params[0]);
            int color = Integer.parseInt(params[1]);
            MainWindow.getInstance().setCenter(new GameView(boardSize, color));
        });

        socketService.send(new Message("LSTONL", ""));

        return playVBox;
    }

    private HBox createSelectBoardSizeBox() {
        Label lblTitle = new Label("Select board size:");
        lblTitle.setFont(Configs.primaryFont(13));

        ComboBox<String> boardSizeComboBox = new ComboBox<>(FXCollections.observableArrayList("9x9", "13x13", "19x19"));
        boardSizeComboBox.setValue("19x19");
        boardSizeComboBox.valueProperty().addListener(((observable, oldValue, newValue) -> {
            selectedBoardSize = Integer.parseInt(newValue.split("x")[0]);
        }));

        HBox selectBoardSizeBox = new HBox();
        selectBoardSizeBox.setAlignment(Pos.CENTER);
        selectBoardSizeBox.getChildren().addAll(lblTitle, boardSizeComboBox);
        selectBoardSizeBox.setSpacing(15);
        selectBoardSizeBox.setMaxWidth(300);

        return selectBoardSizeBox;
    }

    private VBox createOnlineBox() {
        Button btnPlayWithCPU = new Button("Play with computer");
        btnPlayWithCPU.setOnAction(event -> {
            socketService.send(new Message("INVITE", "@CPU" + '\n' + selectedBoardSize + '\n'));
        });

        Label lblTitle = new Label("Challenge another player:");
        lblTitle.setFont(Configs.primaryFont(13));

        onlineListView = new ListView<>();
        onlineListView.setMaxWidth(250);
        onlineListView.setMaxHeight(250);
        onlineListView.setOnMouseClicked(event -> {
            HBox selectedItem = onlineListView.getSelectionModel().getSelectedItem();
            if (selectedItem == null) {
                btnInvite.setDisable(true);
                return;
            }

            boolean available = ((Label) ((HBox) selectedItem.getChildren().get(1)).getChildren().get(1)).getText().equals("Available");
            btnInvite.setDisable(!available);
        });

        btnInvite = new Button("Invite");
        btnInvite.setOnAction(event -> {
            HBox selectedItem = onlineListView.getSelectionModel().getSelectedItem();
            String username = ((Label) ((HBox) selectedItem.getChildren().get(0)).getChildren().get(0)).getText();
            socketService.send(new Message("INVITE", username + '\n' + selectedBoardSize + '\n'));
        });

        VBox onlineBox = new VBox();
        onlineBox.setAlignment(Pos.CENTER);
        onlineBox.setMaxWidth(300);
        onlineBox.setSpacing(10);
        onlineBox.getChildren().addAll(btnPlayWithCPU, lblTitle, onlineListView, btnInvite);

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
