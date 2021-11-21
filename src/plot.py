import seaborn as sns
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import scipy.stats as sts
import subprocess

sns.set_theme()


def plot_burn_in_time(L=20):
    filenames = [
        "burn_in_L_" + str(L) + "_T_1.000000_nonrandom.csv",
        "burn_in_L_" + str(L) + "_T_1.000000_random.csv",
        "burn_in_L_" + str(L) + "_T_2.400000_nonrandom.csv",
        "burn_in_L_" + str(L) + "_T_2.400000_random.csv",
    ]
    temps = [1, 1, 2.4, 2.4]
    spin_orientation = ["ordered", "unordered", "ordered", "unordered"]

    for filename, T, orientation in zip(filenames, temps, spin_orientation):
        df = pd.read_csv("output/" + filename)
        plt.plot(df.N, df.expected_E)
        plt.title(
            "Calculated $<\epsilon>$ for T="
            + str(T)
            + ", L="
            + str(L)
            + ", and "
            + orientation
            + " initial spins"
        )
        plt.xlabel("N")
        plt.ylabel("$<\epsilon>$")
        plt.savefig(
            "plots/burn_in/expected_E" 
            + str(L) 
            + "_T_ " 
            + str(T) 
            + "_" 
            + orientation 
            + ".pdf")
        # plt.show()
        plt.cla()
        plt.plot(df.N, df.expected_M)
        plt.title(
            "Calculated <|m|> for T="
            + str(T)
            + ", L="
            + str(L)
            + ", and "
            + orientation
            + " initial spins"
        )
        plt.xlabel("N")
        plt.ylabel("<|m|>")
        plt.savefig(
            "plots/burn_in/expected_m_abs" 
            + str(L) 
            + "_T_ " 
            + str(T) 
            + "_" 
            + orientation 
            + ".pdf")
        # plt.show()
        plt.cla()


def plot_probability_distribution():
    L = "20"
    for T in ['1.0', '2.1', '2.4']:
        df = pd.read_csv(f"output/samples_L={L}_T={T}.csv")
        plt.title(fr"Estimated probability distribution of $\epsilon$ at T={T}")
        plt.xlabel(r"$\epsilon$")
        plt.ylabel(fr"$p(\epsilon; {T})$")
        plt.hist(df.epsilon, bins="auto", density=True)
        plt.savefig(f"plots/distributions/epsilon_L={L}_T={T}.pdf")
        plt.cla()
        print(f"Variance at T={T}: {df.epsilon.var()}")



def plot_values():
    dfs = {L: pd.read_csv(f"output/values_L={L}.csv") for L in range(20, 160, 20)}
    for value in ["<epsilon>", "<|m|>", "C_v", "chi"]:
        for L in range(40, 160, 20):
            df = dfs[L]
            df.sort_values("T", inplace=True, ignore_index=True)
            plt.plot(df["T"], df[value], label=f"L={L}")
        plt.legend()
        plt.savefig(f"plots/values/plot_{value}.pdf")
        plt.cla()


def estimate_T_inf():
    y_C_v = []
    y_chi = []
    x = []
    for L in range(40, 160, 20):
        df = pd.read_csv(f"output/values_zoom_L={L}.csv")
        argmax_C_v = df.C_v.idxmax()
        argmax_chi = df.chi.idxmax()
        y_C_v.append(df.iloc[argmax_C_v]['T'])
        y_chi.append(df.iloc[argmax_chi]['T'])
        x.append(1 / L)
    linear_fit_C_v = sts.linregress(x, y_C_v)
    linear_fit_chi = sts.linregress(x, y_chi)
    plt.title(r"Observations of $T_c(L)$ against $L^{-1}$ and linear fit to find $T_c(\infty)$")
    fig, axs = plt.subplots(1, 2)
    fig.tight_layout()
    axs[0].scatter(x, y_C_v, label=r"Observed $T_c$")
    estimate_C_v = linear_fit_C_v.intercept
    axs[0].plot([0] + x, estimate_C_v + linear_fit_C_v.slope * np.asarray([0] + x))
    axs[0].plot([0, 0], [estimate_C_v, max(y_C_v)], color="black")
    axs[0].scatter([0], [estimate_C_v], s=40, label=fr"$T_c(\infty) = {estimate_C_v: .3f}$")
    axs[0].legend()
    axs[0].set_title(r"Estimate using $C_v$")

    axs[1].scatter(x, y_chi, label=r"Observed $T_c$")
    estimate_chi = linear_fit_chi.intercept
    axs[1].plot([0] + x, estimate_chi + linear_fit_chi.slope * np.asarray([0] + x))
    axs[1].plot([0, 0], [estimate_chi, max(y_chi)], color="black")
    axs[1].scatter([0], [estimate_chi], s=40, label=fr"$T_c(\infty) = {estimate_chi: .3f}$")
    axs[1].legend()
    axs[1].set_title(r"Estimate using $\chi$")
    
    plt.sca(axs[0])
    plt.yticks([estimate_C_v], [f"{estimate_C_v: .3f}"])
    plt.sca(axs[1])
    plt.yticks([estimate_chi], [f"{estimate_chi: .3f}"])
    print(estimate_C_v)
    plt.savefig("plots/T_inf/estimating_T_inf.pdf")

def main():
    #plot_burn_in_time()
    #plot_probability_distribution()
    plot_values()
    estimate_T_inf()


if __name__ == "__main__":
    main()
