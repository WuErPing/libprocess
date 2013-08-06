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

  // ����
  void open(bool all = true)
  {
    pthread_mutex_lock(&mutex);
    {
      state++; // ״̬+1
      if (all) 
        pthread_cond_broadcast(&cond);
      else 
        pthread_cond_signal(&cond);
    }
    pthread_mutex_unlock(&mutex);
  }

  // �ȴ�
  void wait()
  {
    pthread_mutex_lock(&mutex);
    {
      // �Ⱥ���+1
      waiters++; 
      state_t old = state;
      // �ȴ����ź�״̬�仯
      while (old == state) // ���� spurious wakeup
        pthread_cond_wait(&cond, &mutex);
      // �ѿ��ţ��Ⱥ���-1
      waiters--;
    }
    pthread_mutex_unlock(&mutex);
  }

  // approach �� arrive �������ǽ�����ĺ����ֳ�������
  // ����
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

  // ����
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

  // �뿪
  void leave()
  {
    pthread_mutex_lock(&mutex);
    {
      waiters--;
    }
    pthread_mutex_unlock(&mutex);
  }

	// û�еȺ���
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
