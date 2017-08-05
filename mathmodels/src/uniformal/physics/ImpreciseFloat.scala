package uniformal.physics

case class ImpreciseFloat(value: Float, minus: Float, plus: Float){
  def +(rhs: ImpreciseFloat) = ImpreciseFloat(value + rhs.value, minus + rhs.minus, plus + rhs.plus)
  
  def -(rhs: ImpreciseFloat) = ImpreciseFloat(value - rhs.value, minus + rhs.minus, plus + rhs.plus)
  
  def *(rhs: ImpreciseFloat) = ImpreciseFloat(value * rhs.value, 
      (value * rhs.value)*(relativeMinus+rhs.relativeMinus),  
      (value * rhs.value)*(relativePlus+rhs.relativePlus))
      
  def /(rhs: ImpreciseFloat) = ImpreciseFloat(value / rhs.value, 
      (value / rhs.value)*(relativeMinus+rhs.relativeMinus), 
      (value / rhs.value)*(relativePlus+rhs.relativePlus))
      
  /** two ImpreciseFloats are equal if their intervals intersect */
  def ==(rhs: ImpreciseFloat): Boolean = lowest <= rhs.greatest && greatest >= rhs.lowest
  
  /** two ImpreciseFloats are not equal if their intervals don't intersect */
  def !=(rhs: ImpreciseFloat): Boolean = this == rhs
  
  /** an imprecise float is smaller or equal to another if its greatest possible value is smaller or equal to the other's greatest */
  def <= (rhs: ImpreciseFloat): Boolean = greatest <= rhs.greatest
  
  /** an imprecise float is smaller than another if its greatest possible value is smaller than the other's lowest */
  def <(rhs: ImpreciseFloat): Boolean = this <= rhs && !(this == rhs)
  
  /** an imprecise float is greater than another if the other's greatest possible value is smaller than the its lowest */
  def >(rhs: ImpreciseFloat): Boolean = rhs < this
  
  /** an imprecise float is greater or equal to another if the other's greatest possible value is smaller or equal to the its greatest */
  def >=(rhs: ImpreciseFloat): Boolean = rhs <= this
  
  def relativePlus = plus/value
  
  def relativeMinus = minus/value
  
  def lowest = value - minus
  
  def greatest = value + plus
  
  def abs : ImpreciseFloat = {
    if (value >= 0)
      if (lowest >= 0)
        return this
      else
        return ImpreciseFloat(value, value, plus)
    else
      return ImpreciseFloat(-value, plus, minus).abs
  }
}
