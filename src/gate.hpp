#ifndef GATE_H
#define GATE_H

/* TODO(benh): Provide an implementation directly on-top-of futex's for Linux. */
//#ifdef __linux__
//#else

class Gate
{
public:
  typedef intptr_t state_t;

private:
  int waiters;
  state_t state;
  pthread_mutex_t mutex;
  pthread_cond_t cond;

public:
  Gate() : waiters(0), state(0)
  {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
  }

  ~Gate()
  {
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
  }

  // 开门
  void open(bool all = true)
  {
    pthread_mutex_lock(&mutex);
    {
      state++; // 状态+1
      if (all) 
        pthread_cond_broadcast(&cond);
      else 
        pthread_cond_signal(&cond);
    }
    pthread_mutex_unlock(&mutex);
  }

  // 等待
  void wait()
  {
    pthread_mutex_lock(&mutex);
    {
      // 等候者+1
      waiters++; 
      state_t old = state;
      // 等待开门后状态变化
      while (old == state) // 避免 spurious wakeup
        pthread_cond_wait(&cond, &mutex);
      // 已开门，等候者-1
      waiters--;
    }
    pthread_mutex_unlock(&mutex);
  }

  // approach 与 arrive 看起来是将上面的函数分成了两段
  // 靠近
  state_t approach()
  {
    state_t old;
    pthread_mutex_lock(&mutex);
    {
      waiters++;
      old = state;
    }
    pthread_mutex_unlock(&mutex);
    return old;
  }

  // 到达
  void arrive(state_t old)
  {
    pthread_mutex_lock(&mutex);
    {
      while (old == state) {
        pthread_cond_wait(&cond, &mutex);
      }
      waiters--;
    }
    pthread_mutex_unlock(&mutex);
  }

  // 离开
  void leave()
  {
    pthread_mutex_lock(&mutex);
    {
      waiters--;
    }
    pthread_mutex_unlock(&mutex);
  }

	// 没有等候者
  bool empty()
  {
    bool occupied = true;
    pthread_mutex_lock(&mutex);
    {
      occupied = waiters > 0 ? true : false;
    }
    pthread_mutex_unlock(&mutex);
    return !occupied;
  }
};

//#endif

#endif /* GATE_H */
