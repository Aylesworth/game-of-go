package gameofgo.component;

import gameofgo.common.Configs;
import gameofgo.common.Message;
import gameofgo.service.SocketService;
import javafx.geometry.Pos;
import javafx.scene.control.*;
import javafx.scene.input.MouseEvent;
import javafx.scene.layout.Background;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;

public class SignUpPanel extends VBox {

    private SocketService socketService = SocketService.getInstance();
    private Label lblTitle;
    private Label lblUsername;
    private Label lblPassword;
    private Label lblConfirmPassword;
    private TextField txtUsername;
    private TextField txtPassword;
    private TextField txtConfirmPassword;
    private Label lblMessage;
    private Button btnSignUp;
    private Label lblSignIn;

    public SignUpPanel() {
        double textFieldWidth = 200;

        lblTitle = new Label("SIGN UP");
        lblUsername = new Label("Username");
        lblPassword = new Label("Password");
        lblConfirmPassword = new Label("Confirm password");

        txtUsername = new TextField();
        txtUsername.setPromptText("Username");
        txtUsername.setMaxWidth(textFieldWidth);

        txtPassword = new PasswordField();
        txtPassword.setPromptText("Password");
        txtPassword.setMaxWidth(textFieldWidth);

        txtConfirmPassword = new PasswordField();
        txtConfirmPassword.setPromptText("Confirm password");
        txtConfirmPassword.setMaxWidth(textFieldWidth);

        lblMessage = new Label();
        lblMessage.setTextFill(Color.RED);

        btnSignUp = new Button("Sign up");

        lblSignIn = new Label("Sign in");
        lblSignIn.setTextFill(Color.BLUE);
        lblSignIn.setOnMouseEntered(event -> lblSignIn.setUnderline(true));
        lblSignIn.setOnMouseExited(event -> lblSignIn.setUnderline(false));
        lblSignIn.setOnMouseClicked(event -> MainFrame.getInstance().setCenter(new SignInPanel()));

        setMaxWidth(300);
        setAlignment(Pos.CENTER);
        getChildren().addAll(
                lblTitle,
                lblUsername,
                txtUsername,
                lblPassword,
                txtPassword,
                lblConfirmPassword,
                txtConfirmPassword,
                lblMessage,
                btnSignUp,
                lblSignIn
        );

        btnSignUp.setOnMouseClicked(this::onSubmit);
    }

    private void onSubmit(MouseEvent event) {
        try {
            validate();
        } catch (Exception e) {
            lblMessage.setText(e.getMessage());
            return;
        }


        socketService.on("OK", message -> {
            Alert alert = new Alert(Alert.AlertType.INFORMATION);
            alert.setHeaderText(message.payload());
            alert.showAndWait();

            MainFrame.getInstance().setCenter(new SignInPanel());
        });

        socketService.on("ERROR", message -> lblMessage.setText(message.payload()));

        socketService.send(new Message(
                "SIGNUP",
                txtUsername.getText() + '\n' + txtPassword.getText() + '\n'
        ));
    }

    private void validate() throws Exception {
        lblMessage.setText("");
        txtUsername.setBackground(Background.fill(Color.WHITE));
        txtPassword.setBackground(Background.fill(Color.WHITE));
        txtConfirmPassword.setBackground(Background.fill(Color.WHITE));

        if (txtUsername.getText().isEmpty()) {
            txtUsername.setBackground(Background.fill(Configs.ERROR_COLOR));
            throw new Exception("Username should not be empty");
        }

        if (txtPassword.getText().isEmpty()) {
            txtPassword.setBackground(Background.fill(Configs.ERROR_COLOR));
            throw new Exception("Password should not be empty");
        }

        if (txtPassword.getText().length() < 8) {
            txtPassword.setBackground(Background.fill(Configs.ERROR_COLOR));
            throw new Exception("Password should contain at least 8 characters");
        }

        if (!txtConfirmPassword.getText().equals(txtPassword.getText())) {
            txtConfirmPassword.setBackground(Background.fill(Configs.ERROR_COLOR));
            throw new Exception("Confirm password does not match");
        }
    }
}
