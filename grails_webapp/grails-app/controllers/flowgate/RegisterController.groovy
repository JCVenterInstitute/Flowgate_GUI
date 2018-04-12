package flowgate

import grails.plugin.springsecurity.ui.RegistrationCode

class RegisterController extends grails.plugin.springsecurity.ui.RegisterController {

    def verifyRegistration() {

        String token = params.t

        RegistrationCode registrationCode = token ? RegistrationCode.findByToken(token) : null
        if (!registrationCode) {
            flash.error = message(code: 'spring.security.ui.register.badCode')
            redirect uri: successHandlerDefaultTargetUrl
            return
        }

        def user = uiRegistrationCodeStrategy.finishRegistration(registrationCode)

        if (!user) {
            flash.error = message(code: 'spring.security.ui.register.badCode')
            redirect uri: successHandlerDefaultTargetUrl
            return
        }

        if (user.hasErrors()) {
            // expected to be handled already by ErrorsStrategy.handleValidationErrors
            return
        }

        flash.message = message(code: 'spring.security.ui.register.complete')
        String adminEmail = User.findByUsername('admin').email
        sendMail{
            to adminEmail
            subject "New user registration"
            body "There is a new user registration awaiting confirmation in 'flowgate' app!"
        }
        redirect uri: registerPostRegisterUrl ?: successHandlerDefaultTargetUrl
    }

//    def index() {  }
//    def show() {}
//    def edit() {}
//    def create() {}
}
