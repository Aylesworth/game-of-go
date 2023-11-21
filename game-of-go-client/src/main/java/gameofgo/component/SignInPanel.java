package gameofgo.component;

import gameofgo.common.Configs;
import gameofgo.common.Message;
import gameofgo.common.SessionStorage;
import gameofgo.service.SocketService;
import javafx.geometry.Pos;
import javafx.scene.control.*;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.Background;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;

public class SignInPanel extends VBox {
    private SocketService socketService = SocketService.getInstance();
    private Label lblTitle;
    private Label lblUsername;
    private Label lblPassword;
    private TextField txtUsername;
    private TextField txtPassword;
    private Label lblMessage;
    private Button btnSignIn;
    private Label lblSignUp;

    public SignInPanel() {
        double textFieldWidth = 200;

        lblTitle = new Label("SIGN IN");
        lblUsername = new Label("Username");
        lblPassword = new Label("Password");

        txtUsername = new TextField();
        txtUsername.setPromptText("Username");
        txtUsername.setMaxWidth(textFieldWidth);

        txtPassword = new PasswordField();
        txtPassword.setPromptText("Password");
        txtPassword.setMaxWidth(textFieldWidth);

        lblMessage = new Label();
        lblMessage.setTextFill(Color.RED);

        btnSignIn = new Button("Sign in");

        lblSignUp = new Label("Sign up");
        lblSignUp.setTextFill(Color.BLUE);
        lblSignUp.setOnMouseEntered(event -> lblSignUp.setUnderline(true));
        lblSignUp.setOnMouseExited(event -> lblSignUp.setUnderline(false));
        lblSignUp.setOnMouseClicked(event -> MainFrame.getInstance().setCenter(new SignUpPanel()));

        setMaxWidth(300);
        setAlignment(Pos.CENTER);
        getChildren().addAll(
                lblTitle,
                lblUsername,
                txtUsername,
                lblPassword,
                txtPassword,
                lblMessage,
                btnSignIn,
                lblSignUp
        );

        btnSignIn.setOnMouseClicked(this::onSubmit);

        socketService.on("OK", message -> {
            SessionStorage.setItem("username", txtUsername.getText());
            MainFrame.getInstance().setCenter(new HomePanel());
        });
        socketService.on("ERROR", message -> lblMessage.setText(message.payload()));
    }

    private void onSubmit(MouseEvent event) {
        try {
            validate();
        } catch (Exception e) {
            lblMessage.setText(e.getMessage());
            return;
        }

        socketService.send(new Message(
                "SIGNIN",
                txtUsername.getText() + '\n' + txtPassword.getText() + '\n'
        ));
    }

    private void validate() throws Exception {
        lblMessage.setText("");
        txtUsername.setBackground(Background.fill(Color.WHITE));
        txtPassword.setBackground(Background.fill(Color.WHITE));

        if (txtUsername.getText().isEmpty()) {
            txtUsername.setBackground(Background.fill(Configs.ERROR_COLOR));
            throw new Exception("Please enter your username");
        }

        if (txtPassword.getText().isEmpty()) {
            txtPassword.setBackground(Background.fill(Configs.ERROR_COLOR));
            throw new Exception("Please enter your password");
        }
    }
}
