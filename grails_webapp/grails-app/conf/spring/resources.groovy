// Place your Spring DSL code here
// import org.springframework.security.web.authentication.logout.SecurityContextLogoutHandler

beans = {
//    securityContextLogoutHandler(SecurityContextLogoutHandler) { invalidateHttpSession = false }
    userDetailsService(flowgate.CustomUserDetailsService) {
        grailsApplication = ref('grailsApplication')
    }
}