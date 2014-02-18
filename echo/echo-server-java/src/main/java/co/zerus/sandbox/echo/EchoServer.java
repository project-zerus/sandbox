package co.zerus.sandbox.echo;

import co.zerus.sandbox.echo.thrift.Echo;
import org.apache.thrift.TException;
import org.apache.thrift.async.AsyncMethodCallback;
import org.apache.thrift.server.THsHaServer;
import org.apache.thrift.server.TServer;
import org.apache.thrift.transport.TNonblockingServerSocket;
import org.apache.thrift.transport.TNonblockingServerTransport;
import org.apache.thrift.transport.TTransportException;

import java.util.concurrent.Executors;

public class EchoServer implements Echo.AsyncIface {

    @Override
    public void echo(String msg, AsyncMethodCallback resultHandler) throws TException {
        resultHandler.onComplete(msg);
    }

    public static void main(String[] args) throws Exception {
        try {
            EchoServer echoServer = new EchoServer();
            Echo.AsyncProcessor processor = new Echo.AsyncProcessor(echoServer);
            TNonblockingServerTransport serverTransport = new TNonblockingServerSocket(9000);
            TServer server = new THsHaServer(new THsHaServer.Args(serverTransport)
                    .processor(processor)
                    .executorService(Executors.newFixedThreadPool(20)));
            server.serve();
        } catch (TTransportException e) {
            e.printStackTrace();
        }
    }

}

