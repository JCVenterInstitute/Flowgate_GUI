package flowgate

import com.sun.jersey.api.client.ClientHandlerException
import grails.core.GrailsApplication
import grails.plugin.springsecurity.annotation.Secured
import grails.transaction.Transactional

import static org.springframework.http.HttpStatus.*

@Secured(["IS_AUTHENTICATED_FULLY"])
class TransformationController {

    static allowedMethods = [save: "POST", update: "PUT", delete: "DELETE", axSelectAllFcs: "GET", axUnselectAllFcs: "GET", d3data: "GET", del: ["DELETE", "GET"]]

    def springSecurityService
    def genePatternService
    def restUtilsService
    def utilsService
    def scheduledTaskService

    GrailsApplication grailsApplication

    def index(Integer max) {
        params.max = Math.min(max ?: 10, 100)
        def transformationParameterList = TransformationParameter.findAllByIsPredefined(true)

        if (transformationParameterList) {
            session.removeAttribute('firstTransformation')
            respond transformationParameterList, model: [transformationParameterCount: TransformationParameter.count()]
        } else {
            session.firstTransformation = true;
            redirect action: 'create'
        }
    }

    def create() {
        TransformationParameter transformationParameter = new TransformationParameter(params)
        if (session.firstTransformation) {
            flash.message = "Please create your first transformation parameter.";
            flash.firstTransformation = "Please create your first transformation parameter.";
        }
        respond transformationParameter
    }

    @Transactional
    def save(TransformationParameter transformationParameter) {
        try {
            def transformType = transformationParameter.transformType

            switch (transformType) {
                case "linear":
                    def a = params?.transform_linear_a
                    def t = params?.transform_linear_t
                    transformationParameter.parameterValues = "{\"a\":$a,\"t\":$t}"
                    break;
                case "log":
                    def m = params?.transform_log_m
                    def t = params?.transform_log_t
                    transformationParameter.parameterValues = "{\"m\":$m,\"t\":$t}"
                    break;
                case "logicle":
                    def a = params?.transform_logicle_a
                    def t = params?.transform_logicle_t
                    def m = params?.transform_logicle_m
                    def w = params?.transform_logicle_w
                    transformationParameter.parameterValues = "{\"a\":$a,\"t\":$t,\"m\":$m,\"w\":$w}"
                    break;
            }

            transformationParameter.isPredefined = true
            transformationParameter.validate()
            if (transformationParameter == null) {
                notFound()
                return
            }
            if (transformationParameter.hasErrors()) {
                //transactionStatus.setRollbackOnly()
                respond transformationParameter.errors, view: 'create', model: [transformationParameter: transformationParameter]
                return
            }
            transformationParameter.save flush: true
            request.withFormat {
                form multipartForm {
                    flash.message = message(code: 'default.created.message', args: [message(code: 'transformationParameter.label', default: 'transformationParameter:'), transformationParameter.transformName])
                    redirect action: 'index', params: [eId: params?.eId]
                }
                '*' { respond analysis, [status: CREATED] }
            }
        } catch (ClientHandlerException e) {
            flash.error = "Error occured while creating the transformation parameter."
            redirect action: 'create', model: [transformationParameter: transformationParameter]
            return
        }
    }

    def edit(TransformationParameter transformationParameter) {
        respond transformationParameter
    }

    @Transactional
    def update(Analysis analysis) {
        if (analysis == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }
//        analysis.experiment = Experiment.get(params?.experiment)
//        analysis.user = User.get(params?.user)
        analysis.validate()

        if (analysis.hasErrors()) {
            //transactionStatus.setRollbackOnly()
            analysis.experiment = Experiment.get(params?.experimentId)
            analysis.experiment.project.attach()
            analysis.user.attach()
            respond analysis, view: 'edit'
            return
        }

        analysis.save flush: true

        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.updated.message', args: [message(code: 'analysis.label', default: 'Analysis'), analysis.id])
//                redirect analysis
                redirect controller: 'experiment', action: 'index', params: [eId: analysis?.experiment?.id]
            }
            '*' { respond analysis, [status: OK] }
        }
    }

    @Transactional
    def delete(Analysis analysis) {

        if (analysis == null) {
            render(contentType: "text/json") {
                success false
                msg "Analysis not found!"
            }

            return
        }

        analysis.delete flush: true

        render(contentType: "text/json") {
            success true
        }
    }

    @Transactional
    def del(Analysis analysis) {
//        TODO check if delete on server should be done as well
        if (analysis == null) {
            //transactionStatus.setRollbackOnly()
            notFound()
            return
        }
//        analysis.delete flush:true
        analysis.analysisStatus = -2
        analysis.save flush: true

        request.withFormat {
            /*
            DOES NOT WORK!
            html {
                flash.message = message(code: 'default.deleted.message', args: [message(code: 'analysis.label', default: 'Analysis'), analysis.id])
                redirect action:"index", method:"GET", params: [eId: analysis?.experiment?.id]
            }
            */
            form multipartForm {
                flash.message = message(code: 'default.deleted.message', args: [message(code: 'analysis.label', default: 'Analysis'), analysis.id])
                redirect action: "index", method: "GET", params: [eId: analysis?.experiment?.id]
            }
            '*' {
//                render status: NO_CONTENT
                flash.message = message(code: 'default.deleted.message', args: [message(code: 'analysis.label', default: 'Analysis'), analysis.id])
                redirect action: "index", method: "GET", params: [eId: analysis?.experiment?.id]
            }
        }
    }

    protected void notFound() {
        request.withFormat {
            form multipartForm {
                flash.message = message(code: 'default.not.found.message', args: [message(code: 'analysis.label', default: 'Analysis'), params.id])
                redirect action: "index", method: "GET"
            }
            '*' { render status: NOT_FOUND }
        }
    }
}
