file = 'Core/Src/main.c'

rcc1 = '''
  /**
   * 先将时钟源选择为内部时钟
   * (怎么让CubeMX Generate的时候不删除这一段呢)
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }

'''

rcc2 = '''
  /**
   * 禁用内部高速时钟
   * (怎么让CubeMX Generate的时候不删除这一段呢)
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
'''
with open(file, 'r', encoding='utf-8')as f:
    value: str = f.read()

rcc1_index = value.index('  /** Initializes the RCC Oscillators according to the specified parameters')
value = value[0:rcc1_index] + rcc1 + value[rcc1_index:len(value)]

rcc2_index = value.index('}\n\n/* USER CODE BEGIN 4 */')
value = value[0:rcc2_index] + rcc2 + value[rcc2_index:len(value)]

with open(file, 'w+', encoding='utf-8')as f:
    f.write(value)
