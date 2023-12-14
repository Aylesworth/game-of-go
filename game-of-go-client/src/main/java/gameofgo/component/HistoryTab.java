package gameofgo.component;

import gameofgo.common.*;
import gameofgo.service.SocketService;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.ScrollPane;
import javafx.scene.control.Tab;
import javafx.scene.layout.Background;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.shape.Circle;
import javafx.scene.text.FontWeight;
import javafx.scene.text.TextAlignment;

import java.time.Instant;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.time.format.DateTimeFormatter;

public class HistoryTab extends Tab {
    private final SocketService socketService = SocketService.getInstance();

    public HistoryTab() {
        super("History");
        this.setOnSelectionChanged(event -> {
            if (this.isSelected()) {
                this.setContent(createContent());
            }
        });
    }

    private Node createContent() {
        Label lblTitle = new Label("HISTORY");
        lblTitle.setFont(Configs.primaryFont(FontWeight.BOLD, 24));

        VBox historyVBox = new VBox(20);
        historyVBox.setAlignment(Pos.CENTER);

        VBox wrapperVBox = new VBox(40, lblTitle, historyVBox);
        wrapperVBox.setAlignment(Pos.TOP_CENTER);
        wrapperVBox.setPadding(new Insets(20));
        wrapperVBox.prefWidthProperty().bind(MainWindow.getInstance().widthProperty().subtract(20));
        wrapperVBox.prefHeightProperty().bind(MainWindow.getInstance().heightProperty().subtract(30));
        ScrollPane scrollPane = new ScrollPane(wrapperVBox);

        socketService.on("HISTRY", message -> {
            historyVBox.getChildren().clear();
            for (String record : message.payload().split("\n")) {
                String[] params = record.split(" ");
                GameRecord gameRecord = new GameRecord(
                        params[0],
                        Integer.parseInt(params[1]),
                        params[2],
                        params[3],
                        Double.parseDouble(params[4]),
                        Double.parseDouble(params[5]),
                        Long.parseLong(params[6])
                );
                historyVBox.getChildren().add(createHistoryItem(gameRecord));
            }
        });

        socketService.send(new Message("HISTRY", ""));

        return scrollPane;
    }

    private Node createHistoryItem(GameRecord gameRecord) {
        int playerColor = gameRecord.blackPlayer().equals(SessionStorage.getItem("username")) ? 1 : 2;

        Label lblGameId = new Label('#' + gameRecord.id());
        lblGameId.setFont(Configs.primaryFont(13));

        String opponent = playerColor == 1 ? gameRecord.whitePlayer() : gameRecord.blackPlayer();
        opponent = opponent.startsWith("@") ? opponent.substring(1) : opponent;
        Label lblOpponent = new Label("vs. " + opponent);
        lblOpponent.setFont(Configs.primaryFont(13));

        LocalDateTime time = LocalDateTime.ofInstant(
                Instant.ofEpochSecond(gameRecord.time()),
                ZoneId.systemDefault()
        );
        String formattedTime = DateTimeFormatter.ofPattern("HH:mm dd/MM/yyyy").format(time);
        Label lblTime = new Label(formattedTime);
        lblTime.setFont(Configs.primaryFont(13));

        HBox headerHBox = new HBox(lblGameId, lblOpponent, lblTime);
        headerHBox.setSpacing(200);
        headerHBox.setAlignment(Pos.CENTER);

        double circleSize = 25;
        Circle stoneSymbol = new Circle(circleSize, circleSize, circleSize);
        stoneSymbol.setFill(playerColor == 1 ? Color.BLACK : Color.WHITE);

        Label lblBoardSize = new Label(gameRecord.boardSize() + "x" + gameRecord.boardSize());
        lblBoardSize.setFont(Configs.primaryFont(12));

        VBox smallVBox = new VBox(5, stoneSymbol, lblBoardSize);
        smallVBox.setAlignment(Pos.CENTER);

        String result;
        Color resultTextFill;
        if ((playerColor == 1 && gameRecord.blackScore() > gameRecord.whiteScore())
                || (playerColor == 2 && gameRecord.whiteScore() > gameRecord.blackScore())) {
            result = "VICTORY";
            resultTextFill = Color.BLUEVIOLET;
        } else {
            result = "DEFEAT";
            resultTextFill = Color.DARKRED;
        }
        Label lblResult = new Label(result);
        lblResult.setFont(Configs.primaryFont(24));
        lblResult.setTextFill(resultTextFill);
        lblResult.setTextAlignment(TextAlignment.CENTER);
        lblResult.setMinWidth(100);

        Label lblScores = new Label("Black score: %.1f\nWhite score: %.1f"
                .formatted(gameRecord.blackScore(), gameRecord.whiteScore()));
        lblScores.setFont(Configs.primaryFont(13));
        lblScores.setTextAlignment(TextAlignment.LEFT);

        Button btnReplay = new Button("Replay");

        HBox contentHBox = new HBox(smallVBox, lblResult, lblScores, btnReplay);
        contentHBox.setSpacing(130);
        contentHBox.setAlignment(Pos.CENTER);
        contentHBox.setPadding(new Insets(0, 0, 20, 0));

        VBox mainVBox = new VBox(5, headerHBox, contentHBox);
        mainVBox.setPadding(new Insets(10));
        mainVBox.setBackground(Background.fill(Color.BEIGE));
        mainVBox.setMaxWidth(720);

        btnReplay.setOnAction(event -> {
            socketService.on("REPLAY", message -> {
                String[] params = message.payload().split("\n");
                String log = params[0];
                String blackTerritory = params[1];
                String whiteTerritory = params[2];

                GameReplay gameReplay = new GameReplay(
                        gameRecord.boardSize(),
                        playerColor,
                        log,
                        gameRecord.blackScore(),
                        gameRecord.whiteScore(),
                        blackTerritory.split(" "),
                        whiteTerritory.split(" ")
                );
                MainWindow.getInstance().next(new ReplayView(gameReplay));
            });

            socketService.send(new Message("REPLAY", gameRecord.id() + '\n'));
        });

        return mainVBox;
    }
}
