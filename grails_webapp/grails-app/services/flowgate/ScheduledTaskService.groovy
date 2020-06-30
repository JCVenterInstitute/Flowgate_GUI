package flowgate

import org.springframework.scheduling.annotation.Scheduled

class ScheduledTaskService {


    def springSecurityService
    def utilsService

    boolean lazyInit = false    // <--- this is important

    def jobList = []

    @Scheduled(fixedRate = 10000L) // 10 seconds = 10000L
    def checkTaskResults() {
        if(jobList.size() > 0) {
            println "Scheduled Service: session jobList ${jobList} "
            log.info 'Executing scheduled job...'
            utilsService.checkJobStatus(jobList)
        }
    }

}
