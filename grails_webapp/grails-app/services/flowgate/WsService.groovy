package flowgate

import grails.converters.JSON
import grails.plugin.springwebsocket.WebSocket
import org.springframework.messaging.handler.annotation.SendTo

class WsService implements WebSocket{

    @SendTo("/app/taskChange")
    protected String tcMsg(String mymsg) {
        println "trying to send something"
        String sendStr = ([msg: 'task status change', jobNo: mymsg ] as JSON).toString()
        brokerMessagingTemplate.convertAndSend "/topic/taskChange", sendStr
    }

}
