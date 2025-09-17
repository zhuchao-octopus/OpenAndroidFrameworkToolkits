package com.zhuchao.android.fbase;

import static com.zhuchao.android.fbase.FileUtils.EmptyString;
import static com.zhuchao.android.fbase.FileUtils.MD5;

import com.zhuchao.android.fbase.eventinterface.TaskCallback;

import java.util.Collection;
import java.util.concurrent.atomic.AtomicInteger;

public class TTaskThreadPool extends ObjectList implements TaskCallback {
    private final String TAG = "TaskThreadPool";
    private int maxThreadCount = 200;
    private final int minThreadCount = 1;
    private AtomicInteger taskCounter = new AtomicInteger(0);
    ///private List<PTask> pTaskList_Ok = null;
    private final String ANONYMOUS_NAME = "anonymous-default";
    ///private boolean autoFreeRemove = false;
    ///private final BlockingQueue<TTask> taskQueue = new LinkedBlockingQueue<>();

    public TTaskThreadPool() {
        super();
    }

    public TTaskThreadPool(int maxThreadCount) {
        super();
        this.maxThreadCount = maxThreadCount;
    }

    public PTask createTask() {
        return createTask("");
    }

    public PTask createTask(String tName) {
        if (EmptyString(tName)) {
            //Random random = new Random();
            tName = ANONYMOUS_NAME + System.currentTimeMillis();//匿名线程
            //MMLog.log(TAG, "only one default PTask name/tag ,name=" + tName);
        }

        if (this.getCount() > maxThreadCount) {
            MMLog.i(TAG, "the current thread count is more than maxThreadCount " + maxThreadCount);
            return null;
        }

        TTask tTask = getTaskByTag(MD5(tName));
        if (tTask != null)//存在直接返回
            return (PTask) tTask;

        PTask pTask = new PTask(tName, this);
        addItem(pTask.getTaskTag(), pTask);
        MMLog.log(TAG, "Create PTask tName = " + pTask.getTaskName());// + ",tag = " + pTask.getTaskTag());
        return pTask;
    }

    private int getTaskCounter() {
        return taskCounter.get();
    }

    public TTask getTaskByName(String tName) {
        if (EmptyString(tName)) return null;
        return (TTask) getObject(MD5(tName));
    }

    public TTask getTaskByTag(String tag) {
        if (EmptyString(tag)) return null;
        return (TTask) getObject(tag);
    }

    public <T> T getObjectByName(String tName) {
        if (EmptyString(tName)) return null;
        return (T) getObject(MD5(tName));
    }

    public boolean addTask(TTask tTask) {
        String tTag = tTask.tTag;
        if (!existObject(tTag)) {
            addItem(tTag, tTask);
            return true;
        }
        return false;
    }

    public boolean addTaskInterface(TTaskInterface tTask) {
        String tTag = tTask.getTaskTag();
        if (!existObject(tTag)) {
            addItem(tTag, tTask);
            return true;
        }
        return false;
    }

    public TTask getIdleTask() {
        Collection<Object> objects = getAllObject();
        for (Object o : objects) {
            TTask tTask = ((TTask) o);
            if (!tTask.isBusy()) return tTask;
        }
        return null;
    }

    public TTask getIdleTaskLock() {
        Collection<Object> objects = getAllObject();
        for (Object o : objects) {
            TTask tTask = ((TTask) o);
            if (!tTask.isBusy()) {
                tTask.lock();
                return tTask;
            }
        }
        return null;
    }

    public TTask getNewTask(String tName) {
        if (getTaskByName(tName) != null)
            return createTask(tName + System.currentTimeMillis());
        else
            return createTask(tName);
    }

    public TTask getAvailableTask(String tName) {
        TTask tTask = getIdleTask();
        if (tTask != null) {
            tTask.resetAll();
            return tTask;
        } else return getNewTask(tName);
    }

    public TTask getAvailableTaskLock(String tName) {
        TTask tTask = getIdleTaskLock();
        if (tTask != null) {
            tTask.resetAll();
        } else {
            tTask = getNewTask(tName);
            tTask.lock();
        }
        return tTask;
    }

    public synchronized TTask getSingleTaskFor(String tName) {
        TTask tTask = getTaskByName(tName);
        if (tTask == null) {
            return createTask(tName);
        }
        return tTask;
    }

    public synchronized TTask getSingleTaskLock(String tName) {//主题任务是个异步任务，需要等待
        TTask tTask = getTaskByName(tName);
        if (tTask == null) {
            tTask = createTask(tName);
        }
        if (tTask.isBusy()) return tTask;//无法锁定
        tTask.lock();//主题任务是个异步任务，需要等待
        return tTask;
    }

    public void deleteTaskInterface(TTaskInterface tTask) {
        String tTag = tTask.getTaskTag();
        delete(tTag);
        TTaskInterface tTaskInterface = getTaskByTag(tTag);
        //if (tTaskInterface != null)
        //    MMLog.log(TAG, "delete task tag = " + tTag + ",invokedCount = " + tTaskInterface.getInvokedCount());
        //else
        //    MMLog.log(TAG, "delete task tag = " + tTag);
    }

    public void deleteTask(String tag) {
        delete(tag);
        TTask tTask = getTaskByTag(tag);
        ///if (tTask != null)
        ///    MMLog.log(TAG, "delete task tag = " + tag + ",invokedCount = " + tTask.invokedCount);
        ///else
        ///    MMLog.log(TAG, "delete task tag = " + tag);
    }

    public void deleteTask(TTask tTask) {
        if (tTask == null) return;
        delete(tTask.getTaskTag());
        //MMLog.log(TAG, "delete task tag = " + tTask.getTaskTag() + ",invokedCount = " + tTask.invokedCount);
    }

    public void free() {
        Collection<Object> objects = getAllObject();
        for (Object o : objects) {
            ((PTask) o).freeFree();
        }
        this.clear();
    }

    @Override
    public void onEventTaskFinished(Object obj, int status) {
        TTask tTask = ((TTask) obj);
        if (existObject(tTask.tTag)) {
            MMLog.log(TAG, "pTask finished tTag = " + tTask.tTag + ",total:" + getCount() + ",completed:" + taskCounter);

            if (getTaskCounter() == getCount()) {
                if (tTask.getCallBackHandler() != null) {
                    tTask.getCallBackHandler().onEventTaskFinished(this, DataID.TASK_STATUS_FINISHED_ALL);//池中所有任务完成
                }
            }

            if (tTask.getTaskName().contains(ANONYMOUS_NAME)) {
                MMLog.log(TAG, "free anonymous task name = " + tTask.getTaskName());
                //free();//自动清除匿名线程
                tTask.freeFree();
                deleteTask(tTask);
            }

        } else {
            MMLog.log(TAG, "onEventTask No PTask object in pool,tag = " + tTask.tTag);
        }
    }

    class PTask extends TTask implements TaskCallback {
        //private final String TAG = "PTask";
        public PTask(String tName, TaskCallback threadPoolCallback) {
            super(tName, null);
            setThreadPoolCallback(this);//这里没有调用线程池的call back
        }

        @Override
        public void onEventTaskFinished(Object obj, int status) {
            handlerThreadPool();
        }

        private void handlerThreadPool() {
            if (existObject(tTag)) {
                //MMLog.log(TAG, "invoke TTask demon tTag = " + tTag);
                //super.run(); //执行父类 TTask
                /*//最终结束任务，从任务池中清除掉
                //TTaskThreadPool.delete(this.tTag);结束不清除，等待
                //MMLog.log(TAG, "PTask complete successfully,remove from task pool tag = " + tTag);
                */
                //isKeeping == false 后到这里
                taskCounter.incrementAndGet();//完成的任务计数

                if (getTaskCounter() == getCount()) {
                    doCallBackHandle(DataID.TASK_STATUS_FINISHED_ALL);////池中所有任务完成
                }
                if (this.getTaskName().contains(ANONYMOUS_NAME)) {
                    MMLog.log(TAG, "free anonymous task name = " + this.getTaskName());
                    freeFree();//释放线程资源
                    deleteTask(this);
                } else if (this.isAutoFreeRemove()) {
                    MMLog.log(TAG, "auto Free " + this.getTaskName());
                    freeFree();//释放线程资源，释放资源导致前端无法获得异步任务的数据
                    deleteTask(this);
                } else if (this.isAutoRemove()) {
                    MMLog.log(TAG, "auto remove " + this.getTaskName());
                    deleteTask(this);
                }
                MMLog.log(TAG, "pTask finish tName = " + tName + " invoked:" + invokedCount + " pool:" + getTaskCounter() + "/" + getCount());
            } else {
                MMLog.log(TAG, "not found PTask object in pool,break tag = " + tTag);
            }
        }
    }// class PTask extends TTask implements TaskCallback {

}


