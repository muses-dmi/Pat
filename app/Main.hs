{- LANGUAGE MultiParamTypeClasses -}
{- LANGUAGE FunctionalDependencies -}
{- LANGUAGE TypeSynonymInstances -}
{- LANGAUGE FlexibleInstances -}
module Main where

import Lib

import Control.Monad.Identity
import Control.Monad.State

import Control.Applicative (empty)
import Control.Monad (void, when)
import Data.Void
import Data.Char (isAlphaNum, isSpace)
import Data.List (dropWhileEnd, intercalate)
import Data.Text (Text, pack, singleton)

import Control.Monad.Combinators.Expr
import Text.ParserCombinators.Parsec
import Text.ParserCombinators.Parsec.Expr
import Text.ParserCombinators.Parsec.Language

import qualified Text.ParserCombinators.Parsec.Token as Token

import qualified Control.Functor.Constrained as CF
import qualified Control.Category.Hask as Hask

-------------------------------------------------------------------

languageDef = emptyDef { Token.commentStart    = "/*"
                       , Token.commentEnd      = "*/"
                       , Token.commentLine     = "//"
                       , Token.identStart      = letter
                       , Token.identLetter     = alphaNum
                       , Token.reservedNames   = []
                       , Token.reservedOpNames = []
                       }

lexer = Token.makeTokenParser languageDef

identifier = Token.identifier lexer

-------------------------------------------------------------------

trim :: String -> String
trim = dropWhileEnd isSpace . dropWhile isSpace

split :: String -> String -> [String]
split [] delim = [""]
split xs delim
  | length xs >= length delim =
    if take (length delim) xs == delim
    then "" : split (drop (length delim) xs) delim
    else let xs' = split (drop (length delim) xs) delim
         in (take (length delim) xs ++ head xs') : tail xs'
  | otherwise = [xs]

symbol :: String -> Parser String
symbol = Token.symbol lexer

list = do
  char '['
  list <- (many (spaces >> pattern))
  char ']'
  return (PPSeq list)

pattern = list <|> ((identifier <|> (char '-' >> return "-")) >>= return . PPLit)

pats = many (try (spaces >> pattern))

ppr = do
  p <- pats
  spaces
  symbol "|:|"
  PPR (PPSeq p) . PPSeq <$> pats

ppr' = chainl1 pats oo

oo :: Parser ([Pat String] -> [Pat String] -> [Pat String])
oo = do
  spaces
  symbol "|:|"
  return (\p p' -> [PPR (PPSeq p) (PPSeq p')])

goo = makeExprParser (many (spaces >> pattern) >>= return . PPSeq) bOperators <|> pattern

infixExprHelper p f = do
  p
  return (\e e'-> f e e')

--bOperators :: [[Operator Parser (Pat String)]]
bOperators =
  [ [InfixL (infixExprHelper (symbol "-|-") PPM)
    , InfixL (infixExprHelper (symbol "|:|") PPR) ]
  ]

parseString :: String -> Pat String
parseString str = case parse (many (spaces >> pattern)) "" str of
  Left  e -> error $ show e
  Right r -> PPSeq r

parseString' :: String -> [Pat String]
parseString' str = case parse ppr' "" str of
  Left  e -> error $ show e
  Right r -> r

-------------------------------------------------------------------

data Pat a =
  PPLit a
  | PPSeq [Pat a]
  | PPPar [Pat a]
  | PPM (Pat a) (Pat a)
  | PPR (Pat a) (Pat a)
  deriving (Show)

class Event a b | a -> b where
  slient :: a
  elem :: b -> a

instance Event String String where
  slient = "-"
  elem   = id

instance Functor Pat where
  fmap f (PPLit x)  = PPLit (f x)
  fmap f (PPSeq xs) = PPSeq (fmap (fmap f) xs)
  fmap f (PPPar xs) = PPSeq (fmap (fmap f) xs)
  fmap f (PPM xs ys)   = PPM (fmap f xs) (fmap f ys)
  fmap f (PPR xs ys)   = PPR (fmap f xs) (fmap f ys)

expand :: (Event a b) => Int -> Int -> [Pat a] -> [Pat a]
expand leastCM n seq = foldr
  (\a r -> [a] ++ (take ((leastCM `div` n) - 1) (repeat (PPLit (slient)))) ++ r)
  []
  seq

-----------------------------------------------------------------------------------------------------------------------

data Pattern =
    PStr String
  | PSeq [Pattern]
  | PM [Pattern]
  | PR [Pattern]
    deriving (Show)

-- subdivAux' :: Event a b => Pat a -> (Int, [a])
-- subdivAux' (PPLit s) = (1, [s])
-- subdivAux' (PPSeq ss) =
--   let (ns, ss') = unzip (map subdivAux ss)
--       n         = (foldr1 lcm ns) * length ss
--   in  (n, concat (map (extend n (length ss)) ss'))
--  where
--   extend n ln ss = expand n ln ss
--   extend' n ln ss = ss ++ take ((n `div` ln) - length ss) (repeat "-")

-- subdivAux :: Event a b => Pat a -> (Int, [a])
-- subdivAux (PPLit s)  = (1, [s])
-- --subdivAux (PPPar s)  = (1, [s])
-- subdivAux (PPSeq ss) =
--   let (ns, ss') = unzip (map subdivAux ss)
--       n         = (foldr1 lcm ns) * length ss
--   in (n, concat (map (extend n (length ss)) ss'))
--   where
--     extend n ln ss = ss ++ take ((n `div` ln) - length ss) (repeat slient)

subdiv :: Event a b => Pat a -> [Pat a]
subdiv = snd . subdivAux

subdivAux :: Event a b => Pat a -> (Int, [Pat a])
subdivAux (PPLit s)  = (1, [PPLit s])
subdivAux (PPPar s)  = (1, [PPPar s])
subdivAux (PPSeq ss) =
  let (ns, ss') = unzip (map subdivAux ss)
      n         = (foldr1 lcm ns) * length ss
  in (n, concat (map (extend n (length ss)) ss'))
  where
    extend n ln ss = ss ++ take ((n `div` ln) - length ss) (repeat (PPLit slient))

--expand leastCM n seq = foldr (\a r -> [a] ++ (take ((leastCM `div` n) - 1) (repeat "-")) ++ r)  [] seq 

pm :: Event a b => Pat a -> Pat a -> [[Pat a]]
pm (PPSeq l)  (PPSeq r) = 
  let l' = PPSeq (concat (take (length r) (repeat l)))
      r' = PPSeq (concat (take (length l) (repeat r)))
  in  zipWith (\x y -> [x,y]) (subdiv l') (subdiv r')

pr :: Event a b => Pat a -> Pat a -> Pat a
pr l r = 
  let (nl, sl) = subdivAux l
      (nr, sr) = subdivAux r
      llr = lcm nl nr
      el = expand llr nl sl --foldr (\a r -> [a] ++ (take ((llr `div` nl) - 1) (repeat "-")) ++ r)  [] sl 
      er = expand llr nr sr -- foldr (\a r -> [a] ++ (take ((llr `div` nr) - 1) (repeat "-")) ++ r)  [] sr 
  in PPSeq (map PPPar (zipWith (\x y -> [x,y]) el er))

----------------------

--patToTidal :: Pat String -> String
patToTidal (PPSeq [PPR l r]) = 
  pr l r
  --(map toTidal (pr l r)) 
  --intercalate " " (map toTidal (pr l r)) 

toTidal :: [String] -> String
toTidal = intercalate " " . map (\x -> if x == "-" then "~" else x)

main :: IO ()
main = someFunc
