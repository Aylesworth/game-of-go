package gameofgo.component;

import gameofgo.common.Configs;
import gameofgo.common.Message;
import gameofgo.common.SessionStorage;
import gameofgo.service.SocketService;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.control.*;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.Background;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.text.Font;
import javafx.scene.text.TextAlignment;

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
        double fieldWidth = 200;

        lblTitle = new Label("SIGN IN");
        lblTitle.setFont(Configs.primaryFont(20));

        lblUsername = new Label("Username");
        lblUsername.setFont(Configs.primaryFont(14));
        lblUsername.setMinWidth(fieldWidth);

        lblPassword = new Label("Password");
        lblPassword.setFont(Configs.primaryFont(14));
        lblPassword.setMinWidth(fieldWidth);

        txtUsername = new TextField();
        txtUsername.setPromptText("Username");
        txtUsername.setMaxWidth(fieldWidth);

        txtPassword = new PasswordField();
        txtPassword.setPromptText("Password");
        txtPassword.setMaxWidth(fieldWidth);

        lblMessage = new Label();
        lblMessage.setTextFill(Color.RED);

        btnSignIn = new Button("Sign in");

        lblSignUp = new Label("No account? Sign up");
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
        setMargin(lblTitle, new Insets(0,0,10,0));
        setMargin(txtUsername, new Insets(0,0,10,0));
        setMargin(txtPassword, new Insets(0,0,10,0));
        setMargin(btnSignIn, new Insets(0,0,10,0));

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
