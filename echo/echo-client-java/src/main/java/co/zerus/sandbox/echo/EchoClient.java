package co.zerus.sandbox.echo;

import co.zerus.sandbox.echo.thrift.Echo;
import org.apache.commons.pool2.BasePooledObjectFactory;
import org.apache.commons.pool2.PooledObject;
import org.apache.commons.pool2.impl.DefaultPooledObject;
import org.apache.commons.pool2.impl.GenericObjectPool;
import org.apache.commons.pool2.impl.GenericObjectPoolConfig;
import org.apache.thrift.protocol.TBinaryProtocol;
import org.apache.thrift.protocol.TProtocol;
import org.apache.thrift.transport.TFramedTransport;
import org.apache.thrift.transport.TSocket;
import org.apache.thrift.transport.TTransport;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public class EchoClient {

    static int numOfConnections = 300;
    static int numOfThreads = 200;
    static int numOfRequests = 2000000;

    public static void main(String[] args) throws Exception {
        GenericObjectPoolConfig clientPoolConfig = new GenericObjectPoolConfig();
        clientPoolConfig.setMaxTotal(numOfConnections);
        clientPoolConfig.setMaxIdle(numOfConnections);
        clientPoolConfig.setMinIdle(numOfConnections);

        final GenericObjectPool<Echo.Client> clientPool = new GenericObjectPool<Echo.Client>(new BasePooledObjectFactory<Echo.Client>() {
            @Override
            public Echo.Client create() throws Exception {
                TSocket socket = new TSocket("localhost", 9000);
                TTransport transport = new TFramedTransport(socket);
                transport.open();
                TProtocol protocol = new TBinaryProtocol(transport);
                return new Echo.Client(protocol);
            }

            @Override
            public PooledObject<Echo.Client> wrap(Echo.Client obj) {
                return new DefaultPooledObject(obj);
            }
        }, clientPoolConfig);

        long t0 = System.nanoTime();
        ExecutorService executorService = Executors.newFixedThreadPool(numOfThreads);
        for (int i = 0; i < numOfRequests; ++i) {
            executorService.submit(new Runnable() {
                @Override
                public void run() {
                    try {
                        Echo.Client client = clientPool.borrowObject();
                        client.echo("hi");
                        clientPool.returnObject(client);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            });
        }
        executorService.shutdown();
        executorService.awaitTermination(1, TimeUnit.DAYS);
        long t1 = System.nanoTime();

        double duration = (double) (t1 - t0) / 1.0e9;
        double qps = (double) numOfRequests / duration;

        System.out.println("echo qps: " + qps);
    }

}

