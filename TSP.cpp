//
// Created by duanxu on 16-5-5.
//

#include "TSP.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>

extern int N_NODE_COUNT; //城市数量
extern clock_t start_time;

/*
 * 功能： 构造函数
 */
CTsp::CTsp(void)
{
}

/*
 * 功能： 析构函数
 */
CTsp::~CTsp(void)
{
}


/*
 * 功能： 初始化数据
 * 参数： num_node：   节点数
 *       adj_vec：    图信息
 *       g_Distance： 两两节点间的距离
 *       g_Trial：    两两节点之间的信息素
 * 返回： 空
 */
void CTsp::InitData(int num_node, std::vector<EdgeList> adj_vec, double **g_Distance, double **g_Trial)
{
    // 先把最优蚂蚁的路径长度设置成一个很大的值
    m_cBestAnt.m_dbPathLength = DB_MAX;
    N_NODE_COUNT = num_node;
    // 计算两两城市间距离
    double dbTemp = 0.0;
    for (int i = 0; i < N_NODE_COUNT; i++)
    {
        for (int j = 0; j < N_NODE_COUNT; j++)
        {
            g_Distance[i][j] = DB_MAX;
        }
    }
    EdgeList::iterator adj_it;
    for (int i = 0; i < num_node; i++)
    {
        for (adj_it = adj_vec[i].begin(); adj_it != adj_vec[i].end(); adj_it++)
        {
            g_Distance[i][adj_it->to] = adj_it->cost;
        }
    }

    // 初始化环境信息素，先把城市间的信息素设置成一样
    // 这里设置成1.0，设置成多少对结果影响不是太大，对算法收敛速度有些影响
    for (int i = 0; i < N_NODE_COUNT; i++)
    {
        for (int j = 0; j < N_NODE_COUNT; j++)
        {
            g_Trial[i][j] = 1.0;
        }
    }

}

/*
 * 功能： 更新环境信息素
 * 参数： g_Trial：    两两节点之间的信息素
 * 返回： 空
 */
void CTsp::UpdateTrial(double **g_Trial)
{
    // 临时数组，保存各只蚂蚁在两两城市间新留下的信息素
    double **dbTempAry = (double **) malloc(sizeof(double *) * N_NODE_COUNT);
    for (int i = 0; i < N_NODE_COUNT; i++)
        dbTempAry[i] = (double *) malloc(sizeof(double) * N_NODE_COUNT);

    for (int i = 0; i < N_NODE_COUNT; i++)
        for (int j = 0; j < N_NODE_COUNT; j++)
            dbTempAry[i][j] = 0;

    // 计算新增加的信息素,保存到临时数组里
    int m = 0;
    int n = 0;
    for (int i = 0; i < N_ANT_COUNT; i++) // 计算每只蚂蚁留下的信息素
    {
        for (int j = 1; j < m_cAntAry[i].m_nMovedNodeCount; j++)
        {
            m = m_cAntAry[i].m_nPath[j];
            n = m_cAntAry[i].m_nPath[j - 1];
            dbTempAry[n][m] = dbTempAry[n][m] + DBQ / m_cAntAry[i].m_dbPathLength;//城市n->城市m的信息素
        }
    }

    // 更新环境信息素
    for (int i = 0; i < N_NODE_COUNT; i++)
    {
        for (int j = 0; j < N_NODE_COUNT; j++)
        {
            g_Trial[i][j] = g_Trial[i][j] * ROU + dbTempAry[i][j]; // 最新的环境信息素 = 留存的信息素 + 新留下的信息素
        }
    }

    for (int i = 0; i < N_NODE_COUNT; i++)
        free(dbTempAry[i]);
    free(dbTempAry);

}

/*
 * 功能： 查找路径
 * 参数： demand：     必经节点信息
 *       adj_vec：    图信息
 *       g_Distance： 两两节点间的距离
 *       g_Trial：    两两节点之间的信息素
 *       index：      查找的路径编号，两条路径平分10s时间
 * 返回： 空
 */
void CTsp::Search(Demand &demand, std::vector<EdgeList> adj_vec, double **g_Distance, double **g_Trial, int index)
{
    clock_t end_time;

    // 在迭代次数和总运行时间内进行循环
    for (int i = 0; i < N_IT_COUNT; i++)
    {
        int dead_ants = 0;      // 死掉的蚂蚁数量

        // 每只蚂蚁搜索一遍
        for (int j = 0; j < N_ANT_COUNT; j++)
        {
            m_cAntAry[j].Search(demand, adj_vec, g_Distance, g_Trial);
            if (m_cAntAry[j].m_dbPathLength == DB_MAX)
                dead_ants++;
        }

        // 保存最佳结果
        for (int j = 0; j < N_ANT_COUNT; j++)
        {
            if (m_cAntAry[j].m_dbPathLength < m_cBestAnt.m_dbPathLength)
            {
                m_cBestAnt = m_cAntAry[j];
            }
        }

        // 更新环境信息素
        UpdateTrial(g_Trial);

        // 如果时间用完，也退出搜索
        end_time = clock();
        if ((double) (end_time - start_time) / CLOCKS_PER_SEC > (index + 1) * 5 - 1)
            break;
    }

}
