import numpy as np
import matplotlib.pyplot as plt

if __name__ == "__main__":
    T=0.1
    input = np.load("../data/input.npy")
    state_x0 = np.load("../data/state0.npy")
    state_x1 = np.load("../data/state1.npy")
    state_x2 = np.load("../data/state2.npy")

    T = np.linspace(0,(100-1)*T,100)
    print(T)


    plt.plot(T,state_x0)
    plt.show()